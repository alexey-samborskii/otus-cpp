#include "server/HttpSession.hpp"

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/system/system_error.hpp>

#include <chrono>
#include <iostream>
#include <string>
#include <utility>
#include <stdexcept>

namespace net   = boost::asio;
namespace beast = boost::beast;
namespace http  = boost::beast::http;

namespace server
{

namespace
{

using BeastRequest  = http::request<http::string_body>;
using BeastResponse = http::response<http::string_body>;

constexpr auto kIoTimeout = std::chrono::minutes(1);

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

} // namespace

//------------------------------------------------------------------------------

HttpSession::HttpSession(
    tcp::socket           socket,
    CallbackHandleRequest request_handler_cb)
    : stream_(std::move(socket))
    , request_handler_cb_(std::move(request_handler_cb))
{
    if (!request_handler_cb_)
    {
        throw std::invalid_argument(
            "HTTP request handler must not be null");
    }
}

//------------------------------------------------------------------------------

net::awaitable<void> HttpSession::run()
{
    try
    {
        for (;;)
        {
            BeastRequest beast_request;

            stream_.expires_after(kIoTimeout);

            co_await http::async_read(
                stream_,
                buffer_,
                beast_request,
                net::use_awaitable);

            stream_.expires_never();

            auto request = makeHttpRequest(std::move(beast_request));

            auto response =
                co_await request_handler_cb_(std::move(request));

            auto beast_response = makeBeastResponse(response);

            const bool keep_alive = beast_response.keep_alive();

            stream_.expires_after(kIoTimeout);

            co_await http::async_write(
                stream_,
                beast_response,
                net::use_awaitable);

            stream_.expires_never();

            if (!keep_alive)
            {
                break;
            }
        }
    }
    catch (const boost::system::system_error &error)
    {
        if (error.code() != beast::error::timeout &&
            error.code() != http::error::end_of_stream &&
            error.code() != net::error::operation_aborted)
        {
            std::cerr << "[http session] error: "
                      << error.what()
                      << '\n';
        }
    }

    boost::system::error_code error;

    stream_.socket().shutdown(
        tcp::socket::shutdown_both,
        error);

    stream_.socket().close(error);

    co_return;
}

//------------------------------------------------------------------------------

} // namespace server