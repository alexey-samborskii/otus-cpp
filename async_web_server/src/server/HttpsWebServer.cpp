#include "server/HttpsWebServer.hpp"

#include "server/HttpsSession.hpp"

#include <boost/system/system_error.hpp>

#include <iostream>
#include <memory>
#include <utility>

namespace net = boost::asio;

namespace server
{

HttpsWebServer::HttpsWebServer(
    net::io_context       &io_context,
    const tcp::endpoint   &endpoint,
    std::filesystem::path  public_dir,
    net::ssl::context     &ssl_context)
    : acceptor_(io_context),
      public_dir_(std::move(public_dir)),
      ssl_context_(ssl_context)
{
    boost::system::error_code error;

    acceptor_.open(endpoint.protocol(), error);

    if (error)
    {
        throw boost::system::system_error(
            error,
            "acceptor.open");
    }

    acceptor_.set_option(
        net::socket_base::reuse_address(true),
        error);

    if (error)
    {
        throw boost::system::system_error(
            error,
            "acceptor.set_option");
    }

    acceptor_.bind(endpoint, error);

    if (error)
    {
        throw boost::system::system_error(
            error,
            "acceptor.bind");
    }

    acceptor_.listen(
        net::socket_base::max_listen_connections,
        error);

    if (error)
    {
        throw boost::system::system_error(
            error,
            "acceptor.listen");
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
                public_dir_);

            net::co_spawn(
                acceptor_.get_executor(),
                [session]() {
                    return session->run();
                },
                net::detached);
        }
    }
    catch (const boost::system::system_error &error)
    {
        if (error.code() == net::error::operation_aborted)
        {
            co_return;
        }

        std::cerr << "[https server] accept error: "
                  << error.what()
                  << '\n';
    }
}

//------------------------------------------------------------------------------

void HttpsWebServer::stop()
{
    boost::system::error_code error;

    acceptor_.close(error);
}

} // namespace server