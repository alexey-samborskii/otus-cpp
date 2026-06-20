#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include "common/Logger.hpp"

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

#include <functional>
#include <sstream>

namespace server
{

using AwaitableResponse = boost::asio::awaitable<HttpResponse>;

using CallbackHandleRequest =
    std::function<AwaitableResponse(HttpRequest &&request)>;

//------------------------------------------------------------------------------

inline void handleSessionCompletion(std::exception_ptr exception)
{
    if (!exception)
    {
        return;
    }

    try
    {
        std::rethrow_exception(exception);
    }
    catch (const std::exception &error)
    {
        std::ostringstream message;
        message << "[session] unhandled coroutine exception: " << error.what();
        common::logError(message.str());
    }
    catch (...)
    {
        common::logError("[session] unhandled unknown coroutine exception");
    }
}

//------------------------------------------------------------------------------

} // namespace server