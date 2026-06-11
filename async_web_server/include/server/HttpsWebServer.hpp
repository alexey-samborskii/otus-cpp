#pragma once

#include "server/WebServer.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <filesystem>

namespace server
{

class HttpsWebServer final : public WebServer
{
public:
    using tcp = boost::asio::ip::tcp;

public:
    HttpsWebServer(
        boost::asio::io_context  &io_context,
        const tcp::endpoint      &endpoint,
        std::filesystem::path     public_dir,
        boost::asio::ssl::context &ssl_context);

    boost::asio::awaitable<void> acceptLoop() override;

    void stop() override;

private:
    tcp::acceptor                acceptor_;
    std::filesystem::path        public_dir_;
    boost::asio::ssl::context   &ssl_context_;
};

} // namespace server