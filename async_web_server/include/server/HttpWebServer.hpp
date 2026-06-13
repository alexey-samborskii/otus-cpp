#pragma once

#include "server/WebServer.hpp"

#include <utility>

#include <boost/asio.hpp>

#include <memory>

namespace server
{

class HttpRequestHandler;

class HttpWebServer final : public WebServer
{
public:
    using tcp = boost::asio::ip::tcp;

    HttpWebServer(
        boost::asio::io_context             &io_context,
        const tcp::endpoint                 &endpoint,
        std::shared_ptr<HttpRequestHandler>  request_handler);

    boost::asio::awaitable<void> acceptLoop() override;
    void stop() override;

private:
    tcp::acceptor                        acceptor_;
    std::shared_ptr<HttpRequestHandler> request_handler_;
};

} // namespace server
