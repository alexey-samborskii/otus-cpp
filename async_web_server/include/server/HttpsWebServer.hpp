#pragma once

#include "server/WebServer.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <memory>

namespace server
{

class HttpRequestHandler;

class HttpsWebServer final : public WebServer
{
public:
    using tcp = boost::asio::ip::tcp;

    using SslContextPtr =
        std::shared_ptr<boost::asio::ssl::context>;

    HttpsWebServer(
        boost::asio::io_context            &io_context,
        const tcp::endpoint                &endpoint,
        SslContextPtr                       ssl_context,
        std::shared_ptr<HttpRequestHandler> request_handler);

    boost::asio::awaitable<void> acceptLoop() override;
    void stop() override;

private:
    tcp::acceptor                        acceptor_;
    SslContextPtr                       ssl_context_;
    std::shared_ptr<HttpRequestHandler> request_handler_;
};

} // namespace server