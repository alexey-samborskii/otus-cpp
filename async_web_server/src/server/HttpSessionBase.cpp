#include "server/HttpSessionBase.hpp"

#include <boost/asio/error.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/beast/core/error.hpp>

#include <utility>

namespace http  = boost::beast::http;
namespace ssl   = boost::asio::ssl;
namespace net   = boost::asio;
namespace beast = boost::beast;

namespace server
{

//------------------------------------------------------------------------------

HttpSessionBase::HttpSessionBase(CallbackHandleRequest request_handler_cb)
    : request_handler_cb_(std::move(request_handler_cb))
{
    if (!request_handler_cb_)
    {
        throw std::invalid_argument("HTTP request handler must not be null");
    }
}

//------------------------------------------------------------------------------

auto HttpSessionBase::makeHttpRequest(BeastRequest &&request) -> HttpRequest
{
    HttpRequest result;

    result.method       = std::string(request.method_string());
    result.target       = std::string(request.target());
    result.body         = std::move(request.body());
    result.content_type = std::string(request[http::field::content_type]);
    result.keep_alive   = request.keep_alive();
    result.version      = request.version();

    return result;
}

//------------------------------------------------------------------------------

auto HttpSessionBase::makeBeastResponse(const HttpResponse &response)
    -> HttpSessionBase::BeastResponse
{
    BeastResponse result{
        static_cast<http::status>(response.status),
        response.version};

    result.set(http::field::server, "async_task_web_server");

    if (!response.content_type.empty())
    {
        result.set(http::field::content_type, response.content_type);
    }

    for (const auto &[name, value] : response.headers)
    {
        result.set(name, value);
    }

    result.keep_alive(response.keep_alive);
    result.body() = response.body;
    result.prepare_payload();

    return result;
}

//------------------------------------------------------------------------------

bool HttpSessionBase::shouldIgnoreSystemError(
    const boost::system::error_code &error,
    bool                             ignore_ssl_stream_truncated)
{
    if (error == beast::error::timeout ||
        error == http::error::end_of_stream ||
        error == net::error::operation_aborted)
    {
        return true;
    }

    return ignore_ssl_stream_truncated &&
           error == ssl::error::stream_truncated;
}

//------------------------------------------------------------------------------

} // namespace server
