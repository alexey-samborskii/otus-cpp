#pragma once

#include "server/common.hpp"

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

    auto handle(server::HttpRequest request) const
        -> server::AwaitableResponse;

private:
    std::shared_ptr<tasks::TaskService> service_;
};

} // namespace application
