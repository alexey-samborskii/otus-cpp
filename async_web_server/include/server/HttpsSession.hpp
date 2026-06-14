#pragma once

#include "server/common.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl.hpp>

#include <memory>

namespace server
{

class HttpsSession
{
public:
    using tcp = boost::asio::ip::tcp;

    using SslContextPtr =
        std::shared_ptr<boost::asio::ssl::context>;

    HttpsSession(
        tcp::socket           socket,
        SslContextPtr         ssl_context,
        CallbackHandleRequest request_handler_cb);

    boost::asio::awaitable<void> run();

private:
    SslContextPtr ssl_context_;

    boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
    boost::beast::flat_buffer                          buffer_;
    CallbackHandleRequest                              request_handler_cb_;
};

} // namespace server