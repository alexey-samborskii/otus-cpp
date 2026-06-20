#pragma once

#include "server/common.hpp"
#include "server/WebServer.hpp"

#include <utility>

#include <boost/asio.hpp>

#include <memory>

namespace server
{

//------------------------------------------------------------------------------

class HttpWebServer final : public WebServer
{
public:
    using tcp = boost::asio::ip::tcp;

    HttpWebServer(
        boost::asio::io_context &io_context,
        const tcp::endpoint     &endpoint,
        CallbackHandleRequest    request_handler_cb);

    auto acceptLoop() -> boost::asio::awaitable<void> override;
    void stop() override;

private:
    tcp::acceptor         acceptor_;
    CallbackHandleRequest request_handler_cb_;
};

//------------------------------------------------------------------------------

} // namespace server
