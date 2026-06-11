#include "server/HttpResponse.hpp"

#include <utility>

namespace server
{

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
        "text/plain",
        std::move(body),
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
        "text/plain",
        std::move(body),
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
        "text/plain",
        std::move(body),
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
        "text/plain",
        std::move(body),
        keep_alive,
        version};
}

} // namespace server