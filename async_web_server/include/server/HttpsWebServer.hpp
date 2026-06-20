#pragma once

#include "server/common.hpp"
#include "server/WebServer.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <memory>

namespace server
{

//------------------------------------------------------------------------------

class HttpsWebServer final : public WebServer
{
public:
    using tcp = boost::asio::ip::tcp;

    using SslContextPtr = std::shared_ptr<boost::asio::ssl::context>;

    HttpsWebServer(
        boost::asio::io_context &io_context,
        const tcp::endpoint     &endpoint,
        SslContextPtr            ssl_context,
        CallbackHandleRequest    request_handler_cb);

    auto acceptLoop() -> boost::asio::awaitable<void> override;
    void stop() override;

private:
    tcp::acceptor         acceptor_;
    SslContextPtr         ssl_context_;
    CallbackHandleRequest request_handler_cb_;
};

//------------------------------------------------------------------------------

} // namespace server