#include "server/HttpWebServer.hpp"

#include "server/HttpSession.hpp"

#include "common/Logger.hpp"

#include <boost/system/system_error.hpp>

#include <exception>
#include <memory>
#include <sstream>
#include <utility>

namespace net = boost::asio;

namespace server
{

//------------------------------------------------------------------------------

HttpWebServer::HttpWebServer(
    net::io_context      &io_context,
    const tcp::endpoint  &endpoint,
    CallbackHandleRequest request_handler_cb)
    : acceptor_(io_context)
    , request_handler_cb_(std::move(request_handler_cb))
{
    boost::system::error_code error;

    acceptor_.open(endpoint.protocol(), error);
    if (error)
    {
        throw boost::system::system_error(error, "acceptor.open");
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), error);

    if (error)
    {
        throw boost::system::system_error(error, "acceptor.set_option");
    }

    acceptor_.bind(endpoint, error);
    if (error)
    {
        throw boost::system::system_error(error, "acceptor.bind");
    }

    acceptor_.listen(net::socket_base::max_listen_connections, error);

    if (error)
    {
        throw boost::system::system_error(error, "acceptor.listen");
    }
}

//------------------------------------------------------------------------------

net::awaitable<void> HttpWebServer::acceptLoop()
{
    try
    {
        for (;;)
        {
            tcp::socket socket = co_await acceptor_.async_accept(
                net::use_awaitable);

            auto session = std::make_shared<HttpSession>(
                std::move(socket),
                request_handler_cb_);

            net::co_spawn(
                acceptor_.get_executor(),
                [session]() -> net::awaitable<void> {
                    co_await session->run();
                },
                handleSessionCompletion);
        }
    }
    catch (const boost::system::system_error &error)
    {
        if (error.code() == net::error::operation_aborted)
        {
            co_return;
        }

        std::ostringstream message;
        message << "[http server] accept error: " << error.what();
        common::logError(message.str());
    }
    catch (const std::exception &error)
    {
        std::ostringstream message;
        message << "[http server] unexpected exception: " << error.what();
        common::logError(message.str());
    }
    catch (...)
    {
        common::logError("[http server] unknown exception");
    }

    co_return;
}

//------------------------------------------------------------------------------

void HttpWebServer::stop()
{
    boost::system::error_code error;
    acceptor_.close(error);
}

//------------------------------------------------------------------------------

} // namespace server