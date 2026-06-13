#pragma once

#include <string>
#include <utility>
#include <vector>

namespace server
{

struct HttpResponse
{
    using Header = std::pair<std::string, std::string>;

    unsigned            status = 200;
    std::string         content_type;
    std::string         body;
    std::vector<Header> headers;
    bool                keep_alive = false;
    unsigned            version    = 11;

    static HttpResponse ok(
        std::string body,
        std::string content_type,
        bool        keep_alive,
        unsigned    version);

    static HttpResponse created(
        std::string body,
        std::string location,
        bool        keep_alive,
        unsigned    version);

    static HttpResponse noContent(
        bool     keep_alive,
        unsigned version);

    static HttpResponse badRequest(
        std::string body,
        bool        keep_alive,
        unsigned    version);

    static HttpResponse notFound(
        std::string body,
        bool        keep_alive,
        unsigned    version);

    static HttpResponse methodNotAllowed(
        std::string body,
        bool        keep_alive,
        unsigned    version);

    static HttpResponse conflict(
        std::string body,
        bool        keep_alive,
        unsigned    version);

    static HttpResponse internalServerError(
        std::string body,
        bool        keep_alive,
        unsigned    version);
};

} // namespace server
