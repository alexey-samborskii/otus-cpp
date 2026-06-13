#pragma once

#include <boost/asio/awaitable.hpp>

namespace server
{

class WebServer
{
public:
    virtual ~WebServer() = default;

    virtual boost::asio::awaitable<void> acceptLoop() = 0;
    virtual void stop() = 0;
};

} // namespace server
