#include "server/StaticFileHandler.hpp"

#include <fstream>
#include <sstream>

namespace server
{

//------------------------------------------------------------------------------

StaticFileHandler::StaticFileHandler(std::filesystem::path public_dir)
    : public_dir_(std::move(public_dir))
{
}

//------------------------------------------------------------------------------

HttpResponse StaticFileHandler::handle(const HttpRequest &request) const
{
    if (request.method != "GET")
    {
        return HttpResponse::methodNotAllowed();
    }

    std::string target = request.target;

    const auto query_pos = target.find('?');

    if (query_pos != std::string::npos)
    {
        target = target.substr(0, query_pos);
    }

    if (target == "/")
    {
        target = "/index.html";
    }

    if (target.find("..") != std::string::npos)
    {
        return HttpResponse::badRequest();
    }

    if (!target.empty() && target.front() == '/')
    {
        target.erase(target.begin());
    }

    const std::filesystem::path file_path = public_dir_ / target;

    if (!std::filesystem::is_regular_file(file_path))
    {
        return HttpResponse::notFound();
    }

    std::ifstream file(file_path, std::ios::binary);

    if (!file)
    {
        return HttpResponse::internalServerError();
    }

    std::ostringstream body;
    body << file.rdbuf();

    return HttpResponse::ok(
        body.str(),
        contentType(file_path));
}

//------------------------------------------------------------------------------

std::string StaticFileHandler::contentType(const std::filesystem::path &path)
{
    const std::string extension = path.extension().string();

    if (extension == ".html")
    {
        return "text/html";
    }

    if (extension == ".css")
    {
        return "text/css";
    }

    if (extension == ".js")
    {
        return "application/javascript";
    }

    if (extension == ".json")
    {
        return "application/json";
    }

    if (extension == ".png")
    {
        return "image/png";
    }

    if (extension == ".jpg" || extension == ".jpeg")
    {
        return "image/jpeg";
    }

    return "application/octet-stream";
}

//------------------------------------------------------------------------------

} // namespace server