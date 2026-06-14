#pragma once

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>

#include <functional>

namespace server
{

using AwaitableResponse = boost::asio::awaitable<HttpResponse>;

using CallbackHandleRequest =
    std::function<AwaitableResponse(HttpRequest &&request)>;

}