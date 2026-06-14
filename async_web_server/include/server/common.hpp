#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

#include <iostream>
#include <functional>

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
        std::cerr
            << "[session] unhandled coroutine exception: "
            << error.what()
            << '\n';
    }
    catch (...)
    {
        std::cerr
            << "[session] unhandled unknown coroutine exception\n";
    }
}

//------------------------------------------------------------------------------

}