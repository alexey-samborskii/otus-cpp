#pragma once

#include "server/common.hpp"
#include <utility>

#include <boost/asio.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/core/flat_buffer.hpp>

#include <memory>

namespace server
{

class HttpSession
{
public:
    using tcp = boost::asio::ip::tcp;

    HttpSession(
        tcp::socket           socket,
        CallbackHandleRequest request_handler_cb);

    boost::asio::awaitable<void> run();

private:
    boost::beast::tcp_stream  stream_;
    boost::beast::flat_buffer buffer_;
    CallbackHandleRequest     request_handler_cb_;
};

} // namespace server
