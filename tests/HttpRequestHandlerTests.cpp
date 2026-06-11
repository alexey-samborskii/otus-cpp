#include "server/HttpRequest.hpp"
#include "server/HttpRequestHandler.hpp"
#include "server/HttpResponse.hpp"

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

HttpRequest makeGetRequest(
    std::string target)
{
    HttpRequest request;

    request.method     = "GET";
    request.target     = std::move(target);
    request.keep_alive = false;
    request.version    = 11;

    return request;
}

//------------------------------------------------------------------------------

HttpRequest makePostRequest(
    std::string target)
{
    HttpRequest request;

    request.method     = "POST";
    request.target     = std::move(target);
    request.keep_alive = false;
    request.version    = 11;

    return request;
}

} // namespace

//------------------------------------------------------------------------------

TEST(HttpRequestHandlerTests, ReturnsIndexHtmlForRootPath)
{
    const TempPublicDir public_dir;

    public_dir.writeFile(
        "index.html",
        "<html>Hello from index</html>");

    const HttpResponse response = handleHttpRequest(
        public_dir.path(),
        makeGetRequest("/"));

    EXPECT_EQ(response.status, 200);
    EXPECT_EQ(response.content_type, "text/html");
    EXPECT_EQ(response.body, "<html>Hello from index</html>");
}

//------------------------------------------------------------------------------

TEST(HttpRequestHandlerTests, ReturnsExistingStaticFile)
{
    const TempPublicDir public_dir;

    public_dir.writeFile(
        "style.css",
        "body { margin: 0; }");

    const HttpResponse response = handleHttpRequest(
        public_dir.path(),
        makeGetRequest("/style.css"));

    EXPECT_EQ(response.status, 200);
    EXPECT_EQ(response.content_type, "text/css");
    EXPECT_EQ(response.body, "body { margin: 0; }");
}

//------------------------------------------------------------------------------

TEST(HttpRequestHandlerTests, ReturnsNotFoundForMissingFile)
{
    const TempPublicDir public_dir;

    const HttpResponse response = handleHttpRequest(
        public_dir.path(),
        makeGetRequest("/missing.html"));

    EXPECT_EQ(response.status, 404);
    EXPECT_EQ(response.body, "Not Found\n");
}

//------------------------------------------------------------------------------

TEST(HttpRequestHandlerTests, ReturnsMethodNotAllowedForUnsupportedMethod)
{
    const TempPublicDir public_dir;

    public_dir.writeFile(
        "index.html",
        "Hello");

    const HttpResponse response = handleHttpRequest(
        public_dir.path(),
        makePostRequest("/"));

    EXPECT_EQ(response.status, 405);
    EXPECT_EQ(response.body, "Method Not Allowed\n");
}

//------------------------------------------------------------------------------

TEST(HttpRequestHandlerTests, PreservesKeepAliveAndVersion)
{
    const TempPublicDir public_dir;

    public_dir.writeFile(
        "index.html",
        "Hello");

    HttpRequest request;

    request.method     = "GET";
    request.target     = "/";
    request.keep_alive = true;
    request.version    = 11;

    const HttpResponse response = handleHttpRequest(
        public_dir.path(),
        std::move(request));

    EXPECT_EQ(response.status, 200);
    EXPECT_TRUE(response.keep_alive);
    EXPECT_EQ(response.version, 11);
}

//------------------------------------------------------------------------------

TEST(HttpRequestHandlerTests, RejectsInvalidTarget)
{
    const TempPublicDir public_dir;

    const HttpResponse response = handleHttpRequest(
        public_dir.path(),
        makeGetRequest("index.html"));

    EXPECT_EQ(response.status, 400);
    EXPECT_EQ(response.body, "Bad Request\n");
}

//------------------------------------------------------------------------------

TEST(HttpRequestHandlerTests, RejectsPathTraversal)
{
    const TempPublicDir public_dir;

    const HttpResponse response = handleHttpRequest(
        public_dir.path(),
        makeGetRequest("/../secret.txt"));

    EXPECT_EQ(response.status, 400);
    EXPECT_EQ(response.body, "Bad Request\n");
}

} // namespace server