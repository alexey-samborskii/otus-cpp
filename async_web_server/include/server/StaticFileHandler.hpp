#pragma once

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

#include <filesystem>

namespace server
{

class StaticFileHandler
{
public:
    explicit StaticFileHandler(std::filesystem::path public_dir);

    HttpResponse handle(const HttpRequest &request) const;

private:
    std::filesystem::path public_dir_;

    static std::string contentType(const std::filesystem::path &path);
};

} // namespace server