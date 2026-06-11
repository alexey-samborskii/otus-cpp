#pragma once

#include <string>

namespace server
{

struct HttpResponse
{
    unsigned    status       = 200;
    std::string content_type = "text/plain";
    std::string body;
    bool        keep_alive   = false;
    unsigned    version      = 11;

    static HttpResponse ok(
        std::string body,
        std::string content_type = "text/plain",
        bool        keep_alive   = false,
        unsigned    version      = 11);

    static HttpResponse badRequest(
        std::string body = "Bad Request\n",
        bool        keep_alive = false,
        unsigned    version = 11);

    static HttpResponse notFound(
        std::string body = "Not Found\n",
        bool        keep_alive = false,
        unsigned    version = 11);

    static HttpResponse methodNotAllowed(
        std::string body = "Method Not Allowed\n",
        bool        keep_alive = false,
        unsigned    version = 11);

    static HttpResponse internalServerError(
        std::string body = "Internal Server Error\n",
        bool        keep_alive = false,
        unsigned    version = 11);
};

} // namespace server