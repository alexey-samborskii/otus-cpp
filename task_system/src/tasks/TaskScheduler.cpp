#include "tasks/TaskScheduler.hpp"

#include "common/Logger.hpp"
#include "tasks/TaskRepository.hpp"

#include <boost/system/error_code.hpp>

#include <chrono>
#include <exception>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

namespace net = boost::asio;

namespace tasks
{

//------------------------------------------------------------------------------

TaskScheduler::TaskScheduler(net::any_io_executor executor,
                             TaskRepository      &repository)
    : strand_(net::make_strand(std::move(executor)))
    , repository_(repository)
{
}

//------------------------------------------------------------------------------

TaskScheduler::~TaskScheduler()
{
    cancelAllImpl();
}

//------------------------------------------------------------------------------

net::awaitable<void> TaskScheduler::restore()
{
    const std::vector<Task> scheduled_tasks = repository_.findScheduled();

    for (const Task &task : scheduled_tasks)
    {
        co_await schedule(task);
    }
}

//------------------------------------------------------------------------------

net::awaitable<void> TaskScheduler::schedule(Task task)
{
    co_await net::post(strand_, net::use_awaitable);

    scheduleImpl(std::move(task));
}

//------------------------------------------------------------------------------

net::awaitable<void> TaskScheduler::cancel(TaskId id)
{
    co_await net::post(strand_, net::use_awaitable);

    cancelImpl(id);
}

//------------------------------------------------------------------------------

net::awaitable<void> TaskScheduler::runNow(Task task)
{
    task.scheduled_at_ms = currentUnixTimeMs();

    co_await schedule(std::move(task));
}

//------------------------------------------------------------------------------

TaskSchedulerMetrics TaskScheduler::metrics() const
{
    TaskSchedulerMetrics result;

    result.scheduled_tasks = scheduled_tasks_.load(std::memory_order_relaxed);
    result.cancelled_tasks = cancelled_tasks_.load(std::memory_order_relaxed);
    result.completed_tasks = completed_tasks_.load(std::memory_order_relaxed);
    result.failed_tasks    = failed_tasks_.load(std::memory_order_relaxed);
    result.timer_errors    = timer_errors_.load(std::memory_order_relaxed);

    return result;
}

//------------------------------------------------------------------------------

void TaskScheduler::scheduleImpl(Task task)
{
    cancelImpl(task.id);

    const auto scheduled_time = std::chrono::system_clock::time_point{
        std::chrono::milliseconds(task.scheduled_at_ms)};

    auto         timer = std::make_shared<Timer>(strand_, scheduled_time);
    const TaskId id    = task.id;

    timers_[id] = timer;
    scheduled_tasks_.fetch_add(1, std::memory_order_relaxed);

    {
        std::ostringstream message;
        message << "[scheduler] task " << id
                << " scheduled at " << task.scheduled_at_ms;
        common::logInfo(message.str());
    }

    std::weak_ptr<TaskScheduler> weak_self = shared_from_this();

    timer->async_wait(
        [weak_self, id, timer](const boost::system::error_code &error) {
            const std::shared_ptr<TaskScheduler> self = weak_self.lock();

            if (!self)
            {
                return;
            }

            const auto iterator = self->timers_.find(id);

            if (iterator == self->timers_.end() ||
                iterator->second != timer)
            {
                return;
            }

            self->timers_.erase(iterator);

            if (error == net::error::operation_aborted)
            {
                return;
            }

            if (error)
            {
                self->timer_errors_.fetch_add(1, std::memory_order_relaxed);

                std::ostringstream message;
                message << "[scheduler] timer error for task "
                        << id
                        << ": "
                        << error.message();
                common::logError(message.str());
                return;
            }

            net::co_spawn(
                self->strand_,
                [self, id]() -> net::awaitable<void> {
                    co_await self->executeTask(id);
                },
                net::detached);
        });
}

//------------------------------------------------------------------------------

void TaskScheduler::cancelImpl(TaskId id)
{
    const auto iterator = timers_.find(id);

    if (iterator == timers_.end())
    {
        return;
    }

    const std::shared_ptr<Timer> timer = iterator->second;

    timers_.erase(iterator);
    cancelled_tasks_.fetch_add(1, std::memory_order_relaxed);

    boost::system::error_code error;

    timer->cancel(error);

    if (error)
    {
        std::ostringstream message;
        message << "[scheduler] unable to cancel timer for task "
                << id
                << ": "
                << error.message();
        common::logError(message.str());
    }
}

//------------------------------------------------------------------------------

void TaskScheduler::cancelAllImpl()
{
    for (const auto &[id, timer] : timers_)
    {
        boost::system::error_code error;

        timer->cancel(error);

        if (error)
        {
            std::ostringstream message;
            message << "[scheduler] unable to cancel timer for task "
                    << id << ": " << error.message();
            common::logError(message.str());
        }
    }

    timers_.clear();
}

//------------------------------------------------------------------------------

net::awaitable<void> TaskScheduler::executeTask(TaskId id)
{
    if (!repository_.claimForExecution(id))
    {
        co_return;
    }

    try
    {
        const std::optional<Task> task = repository_.findById(id);

        if (!task.has_value())
        {
            co_return;
        }

        {
            std::ostringstream ss;
            ss << "[scheduler] executing task " << task->id << ": "
               << task->title;
            common::logInfo(ss.str());
        }

        net::steady_timer execution_delay(strand_);

        execution_delay.expires_after(std::chrono::milliseconds(250));

        co_await execution_delay.async_wait(net::use_awaitable);

        repository_.setStatus(id, TaskStatus::kCompleted);
        completed_tasks_.fetch_add(1, std::memory_order_relaxed);

        {
            std::ostringstream ss;
            ss << "[scheduler] task " << id << " completed";
            common::logInfo(ss.str());
        }
    }
    catch (const std::exception &error)
    {
        failed_tasks_.fetch_add(1, std::memory_order_relaxed);

        try
        {
            repository_.setStatus(id, TaskStatus::kFailed, error.what());
        }
        catch (const std::exception &status_error)
        {
            std::ostringstream ss;
            ss << "[scheduler] unable to mark task " << id << " as failed: "
               << status_error.what();
            common::logError(ss.str());
        }

        std::ostringstream ss;
        ss << "[scheduler] task " << id << " failed: " << error.what();
        common::logError(ss.str());
    }
}

//------------------------------------------------------------------------------

} // namespace tasks
