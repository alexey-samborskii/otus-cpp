#pragma once

#include <boost/asio/awaitable.hpp>

namespace server
{

//------------------------------------------------------------------------------

class WebServer
{
public:
    virtual ~WebServer() = default;

    virtual auto acceptLoop() -> boost::asio::awaitable<void> = 0;

    virtual void stop() = 0;
};

//------------------------------------------------------------------------------

} // namespace server
