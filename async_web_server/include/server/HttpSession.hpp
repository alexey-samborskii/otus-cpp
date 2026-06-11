#pragma once

#include <boost/asio.hpp>
#include <boost/beast/core/flat_buffer.hpp>

#include <filesystem>

namespace server
{

class HttpSession
{
public:
    using tcp = boost::asio::ip::tcp;

public:
    HttpSession(
        tcp::socket           socket,
        std::filesystem::path public_dir);

    boost::asio::awaitable<void> run();

private:
    tcp::socket              socket_;
    boost::beast::flat_buffer buffer_;
    std::filesystem::path    public_dir_;
};

} // namespace server