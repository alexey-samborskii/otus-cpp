#pragma once

#include "tasks/Task.hpp"

#include <utility>

#include <boost/asio.hpp>

#include <atomic>
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace tasks
{

class TaskRepository;

struct TaskSchedulerMetrics
{
    std::uint64_t scheduled_tasks = 0;
    std::uint64_t cancelled_tasks = 0;
    std::uint64_t completed_tasks = 0;
    std::uint64_t failed_tasks    = 0;
    std::uint64_t timer_errors    = 0;
};

class TaskScheduler : public std::enable_shared_from_this<TaskScheduler>
{
public:
    TaskScheduler(
        boost::asio::any_io_executor executor,
        TaskRepository              &repository);

    ~TaskScheduler();

    boost::asio::awaitable<void> restore();

    boost::asio::awaitable<void> schedule(Task task);

    boost::asio::awaitable<void> cancel(TaskId id);

    boost::asio::awaitable<void> runNow(Task task);

    TaskSchedulerMetrics metrics() const;

private:
    using Timer = boost::asio::system_timer;

    void scheduleImpl(Task task);
    void cancelImpl(TaskId id);
    void cancelAllImpl();

    boost::asio::awaitable<void> executeTask(TaskId id);

private:
    boost::asio::strand<boost::asio::any_io_executor> strand_;
    TaskRepository                                   &repository_;
    std::unordered_map<TaskId, std::shared_ptr<Timer>> timers_;
    std::atomic_uint64_t scheduled_tasks_{0};
    std::atomic_uint64_t cancelled_tasks_{0};
    std::atomic_uint64_t completed_tasks_{0};
    std::atomic_uint64_t failed_tasks_{0};
    std::atomic_uint64_t timer_errors_{0};
};

} // namespace tasks
