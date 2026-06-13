#pragma once

#include <utility>

#include <boost/asio.hpp>
#include <boost/beast/core/flat_buffer.hpp>

#include <memory>

namespace server
{

class HttpRequestHandler;

class HttpSession
{
public:
    using tcp = boost::asio::ip::tcp;

    HttpSession(
        tcp::socket                         socket,
        std::shared_ptr<HttpRequestHandler> request_handler);

    boost::asio::awaitable<void> run();

private:
    tcp::socket                         socket_;
    boost::beast::flat_buffer           buffer_;
    std::shared_ptr<HttpRequestHandler> request_handler_;
};

} // namespace server
