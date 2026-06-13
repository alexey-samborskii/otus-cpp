#include "server/HttpResponse.hpp"

#include <utility>

namespace server
{

//------------------------------------------------------------------------------

HttpResponse HttpResponse::ok(
    std::string body,
    std::string content_type,
    bool        keep_alive,
    unsigned    version)
{
    return HttpResponse{
        200,
        std::move(content_type),
        std::move(body),
        {},
        keep_alive,
        version};
}

//------------------------------------------------------------------------------

HttpResponse HttpResponse::created(
    std::string body,
    std::string location,
    bool        keep_alive,
    unsigned    version)
{
    return HttpResponse{
        201,
        "application/json; charset=utf-8",
        std::move(body),
        {{"Location", std::move(location)}},
        keep_alive,
        version};
}

//------------------------------------------------------------------------------

HttpResponse HttpResponse::noContent(
    bool     keep_alive,
    unsigned version)
{
    return HttpResponse{
        204,
        "",
        "",
        {},
        keep_alive,
        version};
}

//------------------------------------------------------------------------------

HttpResponse HttpResponse::badRequest(
    std::string body,
    bool        keep_alive,
    unsigned    version)
{
    return HttpResponse{
        400,
        "application/json; charset=utf-8",
        std::move(body),
        {},
        keep_alive,
        version};
}

//------------------------------------------------------------------------------

HttpResponse HttpResponse::notFound(
    std::string body,
    bool        keep_alive,
    unsigned    version)
{
    return HttpResponse{
        404,
        "application/json; charset=utf-8",
        std::move(body),
        {},
        keep_alive,
        version};
}

//------------------------------------------------------------------------------

HttpResponse HttpResponse::methodNotAllowed(
    std::string body,
    bool        keep_alive,
    unsigned    version)
{
    return HttpResponse{
        405,
        "application/json; charset=utf-8",
        std::move(body),
        {},
        keep_alive,
        version};
}

//------------------------------------------------------------------------------

HttpResponse HttpResponse::conflict(
    std::string body,
    bool        keep_alive,
    unsigned    version)
{
    return HttpResponse{
        409,
        "application/json; charset=utf-8",
        std::move(body),
        {},
        keep_alive,
        version};
}

//------------------------------------------------------------------------------

HttpResponse HttpResponse::internalServerError(
    std::string body,
    bool        keep_alive,
    unsigned    version)
{
    return HttpResponse{
        500,
        "application/json; charset=utf-8",
        std::move(body),
        {},
        keep_alive,
        version};
}

//------------------------------------------------------------------------------

} // namespace server
