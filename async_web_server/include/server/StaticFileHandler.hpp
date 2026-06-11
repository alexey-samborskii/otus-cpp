#pragma once

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

#include <filesystem>

namespace server
{

class StaticFileHandler
{
public:
    explicit StaticFileHandler(
        std::filesystem::path public_dir);

    HttpResponse handle(
        HttpRequest &&request) const;

private:
    static std::string contentType(
        const std::filesystem::path &path);

    static bool isValidTarget(
        const std::string &target);

    static std::string removeQueryString(
        std::string target);

    std::filesystem::path makeFilePath(
        std::string target) const;

private:
    std::filesystem::path public_dir_;
};

} // namespace server