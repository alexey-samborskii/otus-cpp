#include "server/HttpsWebServer.hpp"

#include "server/HttpsSession.hpp"

#include "common/Logger.hpp"

#include <boost/system/system_error.hpp>

#include <memory>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace net = boost::asio;

namespace server
{

//------------------------------------------------------------------------------

HttpsWebServer::HttpsWebServer(
    net::io_context      &io_context,
    const tcp::endpoint  &endpoint,
    SslContextPtr         ssl_context,
    CallbackHandleRequest request_handler_cb)
    : acceptor_(io_context)
    , ssl_context_(std::move(ssl_context))
    , request_handler_cb_(std::move(request_handler_cb))
{
    if (!ssl_context_)
    {
        throw std::invalid_argument("SSL context must not be null");
    }
    if (!request_handler_cb_)
    {
        throw std::invalid_argument("HTTP request handler must not be null");
    }

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

net::awaitable<void> HttpsWebServer::acceptLoop()
{
    try
    {
        for (;;)
        {
            tcp::socket socket = co_await acceptor_.async_accept(
                net::use_awaitable);

            auto session = std::make_shared<HttpsSession>(
                std::move(socket),
                ssl_context_,
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
        message << "[https server] accept error: " << error.what();
        common::logError(message.str());
    }
    catch (const std::exception &error)
    {
        std::ostringstream message;
        message << "[https server] unexpected exception: " << error.what();
        common::logError(message.str());
    }
    catch (...)
    {
        common::logError("[https server] unknown exception");
    }

    co_return;
}

//------------------------------------------------------------------------------

void HttpsWebServer::stop()
{
    boost::system::error_code error;
    acceptor_.close(error);
}

//------------------------------------------------------------------------------

} // namespace server