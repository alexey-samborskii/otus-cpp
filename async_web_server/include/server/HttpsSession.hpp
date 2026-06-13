#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/ssl.hpp>

#include <memory>

namespace server
{

class HttpRequestHandler;

class HttpsSession
{
public:
    using tcp = boost::asio::ip::tcp;

    using SslContextPtr =
        std::shared_ptr<boost::asio::ssl::context>;

    HttpsSession(
        tcp::socket                         socket,
        SslContextPtr                       ssl_context,
        std::shared_ptr<HttpRequestHandler> request_handler);

    boost::asio::awaitable<void> run();

private:
    SslContextPtr                       ssl_context_;
    boost::beast::ssl_stream<tcp::socket> stream_;
    boost::beast::flat_buffer           buffer_;
    std::shared_ptr<HttpRequestHandler> request_handler_;
};

} // namespace server