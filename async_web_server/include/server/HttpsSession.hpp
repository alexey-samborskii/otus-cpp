#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/flat_buffer.hpp>

#include <filesystem>

namespace server
{

class HttpsSession
{
public:
    using tcp = boost::asio::ip::tcp;

public:
    HttpsSession(
        tcp::socket                socket,
        boost::asio::ssl::context &ssl_context,
        std::filesystem::path      public_dir);

    boost::asio::awaitable<void> run();

private:
    boost::asio::ssl::stream<tcp::socket> stream_;
    boost::beast::flat_buffer             buffer_;
    std::filesystem::path                 public_dir_;
};

} // namespace server