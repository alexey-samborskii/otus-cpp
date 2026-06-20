#include "server/HttpSession.hpp"

#include <boost/beast/core/error.hpp>

#include <utility>

namespace net = boost::asio;

namespace server
{

//------------------------------------------------------------------------------

HttpSession::HttpSession(
    tcp::socket           socket,
    CallbackHandleRequest request_handler_cb)
    : HttpSessionBase(std::move(request_handler_cb))
    , stream_(std::move(socket))
{
}

//------------------------------------------------------------------------------

net::awaitable<void> HttpSession::run()
{
    co_await processRequests(
        stream_,
        stream_,
        "[http session]",
        false);

    boost::system::error_code error;

    stream_.socket().shutdown(tcp::socket::shutdown_both, error);

    stream_.socket().close(error);

    co_return;
}

//------------------------------------------------------------------------------

} // namespace server
