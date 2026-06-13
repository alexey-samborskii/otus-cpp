#pragma once

#include "tasks/Task.hpp"

#include <utility>

#include <boost/asio.hpp>

#include <memory>
#include <unordered_map>

namespace tasks
{

class TaskRepository;

class TaskScheduler : public std::enable_shared_from_this<TaskScheduler>
{
public:
    TaskScheduler(
        boost::asio::any_io_executor executor,
        TaskRepository              &repository);

    boost::asio::awaitable<void> restore();

    boost::asio::awaitable<void> schedule(Task task);

    boost::asio::awaitable<void> cancel(TaskId id);

    boost::asio::awaitable<void> runNow(Task task);

private:
    using Timer = boost::asio::system_timer;

    void scheduleImpl(Task task);
    void cancelImpl(TaskId id);

    boost::asio::awaitable<void> executeTask(TaskId id);

private:
    boost::asio::strand<boost::asio::any_io_executor> strand_;
    TaskRepository                                   &repository_;
    std::unordered_map<TaskId, std::shared_ptr<Timer>> timers_;
};

} // namespace tasks
