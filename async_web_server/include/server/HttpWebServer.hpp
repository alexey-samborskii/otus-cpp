#pragma once

#include "server/WebServer.hpp"

#include <boost/asio.hpp>

#include <filesystem>

namespace server
{

class HttpWebServer final : public WebServer
{
public:
    using tcp = boost::asio::ip::tcp;

public:
    HttpWebServer(
        boost::asio::io_context  &io_context,
        const tcp::endpoint      &endpoint,
        std::filesystem::path     public_dir);

    boost::asio::awaitable<void> acceptLoop() override;

    void stop() override;

private:
    tcp::acceptor         acceptor_;
    std::filesystem::path public_dir_;
};

} // namespace server