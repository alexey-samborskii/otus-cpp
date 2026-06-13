#pragma once

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

#include <boost/asio/awaitable.hpp>

#include <memory>

namespace tasks
{
class TaskService;
}

namespace application
{

class TaskApiHandler
{
public:
    explicit TaskApiHandler(std::shared_ptr<tasks::TaskService> service);

    boost::asio::awaitable<server::HttpResponse> handle(
        server::HttpRequest request) const;

private:
    std::shared_ptr<tasks::TaskService> service_;
};

} // namespace application
