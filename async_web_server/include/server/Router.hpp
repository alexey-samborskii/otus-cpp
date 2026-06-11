#pragma once

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"
#include "server/StaticFileHandler.hpp"

#include <filesystem>

namespace server
{

class Router
{
public:
    explicit Router(
        std::filesystem::path public_dir);

    HttpResponse route(
        HttpRequest &&request) const;

private:
    StaticFileHandler static_file_handler_;
};

} // namespace server