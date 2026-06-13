#pragma once

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

#include <boost/asio/awaitable.hpp>

namespace server
{

class HttpRequestHandler
{
public:
    virtual ~HttpRequestHandler() = default;

    virtual boost::asio::awaitable<HttpResponse> handle(HttpRequest request) = 0;
};

} // namespace server
