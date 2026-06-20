#pragma once

#include "server/HttpSessionBase.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core/tcp_stream.hpp>

#include <utility>

namespace server
{

//------------------------------------------------------------------------------

class HttpSession : private HttpSessionBase
{
private:
    using Base = HttpSessionBase;

public:
    using tcp = boost::asio::ip::tcp;

    HttpSession(tcp::socket socket, CallbackHandleRequest request_handler_cb);

    boost::asio::awaitable<void> run();

private:
    boost::beast::tcp_stream stream_;
};

//------------------------------------------------------------------------------

} // namespace server