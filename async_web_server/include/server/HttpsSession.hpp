#pragma once

#include "server/HttpSessionBase.hpp"

#include <boost/asio/awaitable.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl.hpp>

#include <memory>

namespace server
{

//------------------------------------------------------------------------------

class HttpsSession : private HttpSessionBase
{
private:
    using Base = HttpSessionBase;

public:
    using tcp = boost::asio::ip::tcp;

    using SslContextPtr = std::shared_ptr<boost::asio::ssl::context>;

    HttpsSession(
        tcp::socket           socket,
        SslContextPtr         ssl_context,
        CallbackHandleRequest request_handler_cb);

    boost::asio::awaitable<void> run();

private:
    SslContextPtr ssl_context_;

    boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
};

//------------------------------------------------------------------------------

} // namespace server