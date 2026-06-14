#include "server/HttpsSession.hpp"

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

#include <boost/asio/ssl/error.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/system_error.hpp>

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace net  = boost::asio;
namespace ssl  = boost::asio::ssl;
namespace http = boost::beast::http;

namespace server
{

namespace
{

using BeastRequest  = http::request<http::string_body>;
using BeastResponse = http::response<http::string_body>;

//------------------------------------------------------------------------------

ssl::context &requireSslContext(
    const HttpsSession::SslContextPtr &ssl_context)
{
    if (!ssl_context)
    {
        throw std::invalid_argument(
            "SSL context must not be null");
    }

    return *ssl_context;
}

//------------------------------------------------------------------------------

HttpRequest makeHttpRequest(BeastRequest &&request)
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

BeastResponse makeBeastResponse(const HttpResponse &response)
{
    BeastResponse result{
        static_cast<http::status>(response.status),
        response.version};

    result.set(
        http::field::server,
        "async_task_web_server");

    if (!response.content_type.empty())
    {
        result.set(
            http::field::content_type,
            response.content_type);
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

} // namespace

//------------------------------------------------------------------------------

HttpsSession::HttpsSession(
    tcp::socket           socket,
    SslContextPtr         ssl_context,
    CallbackHandleRequest request_handler_cb)
    : ssl_context_(std::move(ssl_context))
    , stream_(std::move(socket), requireSslContext(ssl_context_))
    , request_handler_cb_(std::move(request_handler_cb))
{
    if (!request_handler_cb_)
    {
        throw std::invalid_argument("HTTP request handler must not be null");
    }
}

//------------------------------------------------------------------------------

net::awaitable<void> HttpsSession::run()
{
    try
    {
        co_await stream_.async_handshake(
            ssl::stream_base::server,
            net::use_awaitable);

        for (;;)
        {
            BeastRequest beast_request;

            co_await http::async_read(
                stream_,
                buffer_,
                beast_request,
                net::use_awaitable);

            auto request = makeHttpRequest(std::move(beast_request));

            auto response = co_await request_handler_cb_(std::move(request));

            auto beast_response = makeBeastResponse(response);

            const bool keep_alive = beast_response.keep_alive();

            co_await http::async_write(
                stream_,
                beast_response,
                net::use_awaitable);

            if (!keep_alive)
            {
                break;
            }
        }

        boost::system::error_code error;

        co_await stream_.async_shutdown(
            net::redirect_error(
                net::use_awaitable,
                error));

        if (error == ssl::error::stream_truncated)
        {
            error.clear();
        }

        if (error)
        {
            std::cerr
                << "[https session] shutdown error: "
                << error.message()
                << '\n';
        }
    }
    catch (const boost::system::system_error &error)
    {
        if (error.code() != http::error::end_of_stream &&
            error.code() != net::error::operation_aborted &&
            error.code() != ssl::error::stream_truncated)
        {
            std::cerr
                << "[https session] error: "
                << error.what()
                << '\n';
        }
    }

    co_return;
}

} // namespace server