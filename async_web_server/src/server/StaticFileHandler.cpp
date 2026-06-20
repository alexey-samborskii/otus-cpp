#include "server/StaticFileHandler.hpp"

#include <fstream>
#include <sstream>
#include <string>
#include <utility>

namespace server
{

//------------------------------------------------------------------------------

StaticFileHandler::StaticFileHandler(std::filesystem::path public_dir)
    : public_dir_(std::move(public_dir))
{
}

//------------------------------------------------------------------------------

HttpResponse StaticFileHandler::handle(HttpRequest request) const
{
    if (request.method != "GET")
    {
        return HttpResponse::methodNotAllowed(
            R"({"error":"Method Not Allowed"})",
            request.keep_alive,
            request.version);
    }

    if (!isValidTarget(request.target))
    {
        return HttpResponse::badRequest(
            R"({"error":"Bad Request"})",
            request.keep_alive,
            request.version);
    }

    const std::filesystem::path file_path = makeFilePath(request.target);

    if (!std::filesystem::exists(file_path) ||
        !std::filesystem::is_regular_file(file_path))
    {
        return HttpResponse::notFound(
            R"({"error":"Not Found"})",
            request.keep_alive,
            request.version);
    }

    std::ifstream file(file_path, std::ios::binary);

    if (!file)
    {
        return HttpResponse::internalServerError(
            R"({"error":"Internal Server Error"})",
            request.keep_alive,
            request.version);
    }

    std::ostringstream body;

    body << file.rdbuf();

    HttpResponse response = HttpResponse::ok(
        body.str(),
        contentType(file_path),
        request.keep_alive,
        request.version);

    response.headers.emplace_back("Cache-Control", "no-cache");

    return response;
}

//------------------------------------------------------------------------------

std::string StaticFileHandler::contentType(const std::filesystem::path &path)
{
    const std::string extension = path.extension().string();

    if (extension == ".htm" || extension == ".html")
    {
        return "text/html; charset=utf-8";
    }

    if (extension == ".css")
    {
        return "text/css; charset=utf-8";
    }

    if (extension == ".js")
    {
        return "application/javascript; charset=utf-8";
    }

    if (extension == ".json")
    {
        return "application/json; charset=utf-8";
    }

    if (extension == ".png")
    {
        return "image/png";
    }

    if (extension == ".jpg" || extension == ".jpeg")
    {
        return "image/jpeg";
    }

    if (extension == ".svg")
    {
        return "image/svg+xml";
    }

    if (extension == ".ico")
    {
        return "image/x-icon";
    }

    if (extension == ".txt")
    {
        return "text/plain; charset=utf-8";
    }

    return "application/octet-stream";
}

//------------------------------------------------------------------------------

bool StaticFileHandler::isValidTarget(const std::string &target)
{
    if (target.empty())
    {
        return false;
    }

    if (target.front() != '/')
    {
        return false;
    }

    if (target.find("..") != std::string::npos)
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------

std::string StaticFileHandler::removeQueryString(std::string target)
{
    const std::string::size_type query_pos = target.find('?');

    if (query_pos != std::string::npos)
    {
        target.erase(query_pos);
    }

    return target;
}

//------------------------------------------------------------------------------

std::filesystem::path StaticFileHandler::makeFilePath(std::string target) const
{
    target = removeQueryString(std::move(target));

    if (target == "/")
    {
        target = "/index.html";
    }

    const std::filesystem::path relative_path = target.substr(1);

    return (public_dir_ / relative_path).lexically_normal();
}

//------------------------------------------------------------------------------

} // namespace server
