#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"
#include "server/StaticFileHandler.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <utility>

namespace server
{

namespace
{

class TempPublicDir
{
public:
    TempPublicDir()
        : path_(
              std::filesystem::temp_directory_path() /
              ("async_web_server_handler_test_" + makeUniqueSuffix()))
    {
        std::filesystem::create_directories(path_);
    }

    ~TempPublicDir()
    {
        std::error_code error;

        std::filesystem::remove_all(
            path_,
            error);
    }

    const std::filesystem::path &path() const
    {
        return path_;
    }

    void writeFile(
        const std::filesystem::path &relative_path,
        const std::string           &content) const
    {
        const std::filesystem::path file_path = path_ / relative_path;

        std::filesystem::create_directories(
            file_path.parent_path());

        std::ofstream file(
            file_path,
            std::ios::binary);

        file << content;
    }

private:
    static std::string makeUniqueSuffix()
    {
        static std::atomic_uint64_t counter{0};

        const auto now = std::chrono::steady_clock::now()
                             .time_since_epoch()
                             .count();

        return std::to_string(now) + "_" +
               std::to_string(counter.fetch_add(1));
    }

private:
    std::filesystem::path path_;
};

//------------------------------------------------------------------------------

HttpRequest makeRequest(
    std::string method,
    std::string target,
    bool        keep_alive = false,
    unsigned    version    = 11)
{
    HttpRequest request;

    request.method     = std::move(method);
    request.target     = std::move(target);
    request.keep_alive = keep_alive;
    request.version    = version;

    return request;
}

//------------------------------------------------------------------------------

void expectHeader(
    const HttpResponse &response,
    const std::string  &expected_name,
    const std::string  &expected_value)
{
    for (const auto &[name, value] : response.headers)
    {
        if (name == expected_name)
        {
            EXPECT_EQ(value, expected_value);
            return;
        }
    }

    FAIL() << "Header was not found: " << expected_name;
}

} // namespace

//------------------------------------------------------------------------------

TEST(StaticFileHandlerTests, ReturnsIndexHtmlForRootPath)
{
    const TempPublicDir public_dir;

    public_dir.writeFile(
        "index.html",
        "<html>Hello from index</html>");

    const StaticFileHandler handler(public_dir.path());
    const HttpResponse response = handler.handle(
        makeRequest("GET", "/"));

    EXPECT_EQ(response.status, 200);
    EXPECT_EQ(response.content_type, "text/html; charset=utf-8");
    EXPECT_EQ(response.body, "<html>Hello from index</html>");
    EXPECT_FALSE(response.keep_alive);
    EXPECT_EQ(response.version, 11);
    expectHeader(response, "Cache-Control", "no-cache");
}

//------------------------------------------------------------------------------

TEST(StaticFileHandlerTests, ReturnsExistingStaticFile)
{
    const TempPublicDir public_dir;

    public_dir.writeFile(
        "styles/style.css",
        "body { margin: 0; }");

    const StaticFileHandler handler(public_dir.path());
    const HttpResponse response = handler.handle(
        makeRequest("GET", "/styles/style.css"));

    EXPECT_EQ(response.status, 200);
    EXPECT_EQ(response.content_type, "text/css; charset=utf-8");
    EXPECT_EQ(response.body, "body { margin: 0; }");
    expectHeader(response, "Cache-Control", "no-cache");
}

//------------------------------------------------------------------------------

TEST(StaticFileHandlerTests, IgnoresQueryStringWhenResolvingFile)
{
    const TempPublicDir public_dir;

    public_dir.writeFile(
        "app.js",
        "console.log('test');");

    const StaticFileHandler handler(public_dir.path());
    const HttpResponse response = handler.handle(
        makeRequest("GET", "/app.js?v=42"));

    EXPECT_EQ(response.status, 200);
    EXPECT_EQ(
        response.content_type,
        "application/javascript; charset=utf-8");
    EXPECT_EQ(response.body, "console.log('test');");
}

//------------------------------------------------------------------------------

TEST(StaticFileHandlerTests, ReturnsNotFoundForMissingFile)
{
    const TempPublicDir public_dir;
    const StaticFileHandler handler(public_dir.path());

    const HttpResponse response = handler.handle(
        makeRequest("GET", "/missing.html", true, 10));

    EXPECT_EQ(response.status, 404);
    EXPECT_EQ(
        response.content_type,
        "application/json; charset=utf-8");
    EXPECT_EQ(response.body, R"({"error":"Not Found"})");
    EXPECT_TRUE(response.keep_alive);
    EXPECT_EQ(response.version, 10);
}

//------------------------------------------------------------------------------

TEST(StaticFileHandlerTests, ReturnsMethodNotAllowedForUnsupportedMethod)
{
    const TempPublicDir public_dir;

    public_dir.writeFile(
        "index.html",
        "Hello");

    const StaticFileHandler handler(public_dir.path());
    const HttpResponse response = handler.handle(
        makeRequest("POST", "/", true, 10));

    EXPECT_EQ(response.status, 405);
    EXPECT_EQ(
        response.content_type,
        "application/json; charset=utf-8");
    EXPECT_EQ(response.body, R"({"error":"Method Not Allowed"})");
    EXPECT_TRUE(response.keep_alive);
    EXPECT_EQ(response.version, 10);
}

//------------------------------------------------------------------------------

TEST(StaticFileHandlerTests, PreservesKeepAliveAndVersion)
{
    const TempPublicDir public_dir;

    public_dir.writeFile(
        "index.html",
        "Hello");

    const StaticFileHandler handler(public_dir.path());
    const HttpResponse response = handler.handle(
        makeRequest("GET", "/", true, 10));

    EXPECT_EQ(response.status, 200);
    EXPECT_TRUE(response.keep_alive);
    EXPECT_EQ(response.version, 10);
}

//------------------------------------------------------------------------------

TEST(StaticFileHandlerTests, RejectsInvalidTarget)
{
    const TempPublicDir public_dir;
    const StaticFileHandler handler(public_dir.path());

    const HttpResponse response = handler.handle(
        makeRequest("GET", "index.html"));

    EXPECT_EQ(response.status, 400);
    EXPECT_EQ(
        response.content_type,
        "application/json; charset=utf-8");
    EXPECT_EQ(response.body, R"({"error":"Bad Request"})");
}

//------------------------------------------------------------------------------

TEST(StaticFileHandlerTests, RejectsPathTraversal)
{
    const TempPublicDir public_dir;
    const StaticFileHandler handler(public_dir.path());

    const HttpResponse response = handler.handle(
        makeRequest("GET", "/../secret.txt"));

    EXPECT_EQ(response.status, 400);
    EXPECT_EQ(
        response.content_type,
        "application/json; charset=utf-8");
    EXPECT_EQ(response.body, R"({"error":"Bad Request"})");
}

} // namespace server