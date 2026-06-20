#include "server/HttpsSession.hpp"

#include "common/Logger.hpp"

#include <boost/asio/redirect_error.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/system/system_error.hpp>

#include <chrono>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace net   = boost::asio;
namespace ssl   = boost::asio::ssl;
namespace beast = boost::beast;

namespace server
{

namespace
{

//------------------------------------------------------------------------------

ssl::context &requireSslContext(const HttpsSession::SslContextPtr &ssl_context)
{
    if (!ssl_context)
    {
        throw std::invalid_argument("SSL context must not be null");
    }

    return *ssl_context;
}

//------------------------------------------------------------------------------

void logHttpsSystemError(
    const boost::system::system_error &error)
{
    if (error.code() == beast::error::timeout ||
        error.code() == net::error::operation_aborted ||
        error.code() == ssl::error::stream_truncated)
    {
        return;
    }

    std::ostringstream message;

    message << "[https session] error: " << error.what();

    common::logError(message.str());
}

} // namespace

//------------------------------------------------------------------------------

HttpsSession::HttpsSession(
    tcp::socket           socket,
    SslContextPtr         ssl_context,
    CallbackHandleRequest request_handler_cb)
    : HttpSessionBase(std::move(request_handler_cb))
    , ssl_context_(std::move(ssl_context))
    , stream_(std::move(socket), requireSslContext(ssl_context_))
{
}

//------------------------------------------------------------------------------

net::awaitable<void> HttpsSession::run()
{
    auto &transport = beast::get_lowest_layer(stream_);

    try
    {
        transport.expires_after(std::chrono::minutes{1});

        co_await stream_.async_handshake(
            ssl::stream_base::server,
            net::use_awaitable);

        transport.expires_never();

        co_await processRequests(
            stream_,
            transport,
            "[https session]",
            true);

        boost::system::error_code shutdown_error;

        transport.expires_after(std::chrono::minutes{1});

        co_await stream_.async_shutdown(
            net::redirect_error(
                net::use_awaitable,
                shutdown_error));

        transport.expires_never();

        if (shutdown_error == ssl::error::stream_truncated ||
            shutdown_error == beast::error::timeout ||
            shutdown_error == net::error::operation_aborted)
        {
            shutdown_error.clear();
        }

        if (shutdown_error)
        {
            std::ostringstream message;

            message << "[https session] shutdown error: "
                    << shutdown_error.message();

            common::logError(message.str());
        }
    }
    catch (const boost::system::system_error &error)
    {
        logHttpsSystemError(error);
    }
    catch (const std::exception &error)
    {
        std::ostringstream message;

        message << "[https session] exception: " << error.what();

        common::logError(message.str());
    }
    catch (...)
    {
        common::logError("[https session] unknown exception");
    }

    boost::system::error_code close_error;

    transport.socket().close(close_error);

    co_return;
}

//------------------------------------------------------------------------------

} // namespace server
