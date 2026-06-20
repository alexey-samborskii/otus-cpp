#include "common/Logger.hpp"
#include "tasks/TaskRepository.hpp"
#include "tasks/TaskScheduler.hpp"

#include <boost/asio.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <exception>
#include <memory>
#include <optional>
#include <string>
#include <utility>

namespace net = boost::asio;

namespace
{

using namespace std::chrono_literals;

class TaskSchedulerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        common::setLogLevel(common::LogLevel::kError);
    }
};

//------------------------------------------------------------------------------

template <typename Awaitable>
void runAwaitable(net::io_context &io_context, Awaitable &&awaitable)
{
    std::exception_ptr async_error;

    net::co_spawn(
        io_context,
        std::forward<Awaitable>(awaitable),
        [&io_context, &async_error](std::exception_ptr error) {
            async_error = error;
            io_context.stop();
        });

    io_context.run();
    io_context.restart();

    if (async_error)
    {
        std::rethrow_exception(async_error);
    }
}

//------------------------------------------------------------------------------

void runFor(net::io_context &io_context, std::chrono::milliseconds duration)
{
    net::steady_timer stop_timer(io_context);

    stop_timer.expires_after(duration);
    stop_timer.async_wait(
        [&io_context](const boost::system::error_code &) {
            io_context.stop();
        });

    io_context.run();
    io_context.restart();
}

//------------------------------------------------------------------------------

tasks::TaskInput makeTaskInput(
    const std::string &title,
    std::int64_t       scheduled_at_ms)
{
    tasks::TaskInput input;

    input.title           = title;
    input.description     = "test description";
    input.scheduled_at_ms = scheduled_at_ms;

    return input;
}

//------------------------------------------------------------------------------

TEST_F(TaskSchedulerTest, CompletesScheduledTask)
{
    net::io_context io_context;

    tasks::TaskRepository repository(":memory:");

    auto scheduler = std::make_shared<tasks::TaskScheduler>(
        io_context.get_executor(),
        repository);

    const tasks::TaskInput input = makeTaskInput(
        "scheduled task",
        tasks::currentUnixTimeMs() + 50);

    const tasks::Task task = repository.create(input);

    runAwaitable(io_context, scheduler->schedule(task));
    runFor(io_context, 700ms);

    const std::optional<tasks::Task> stored_task = repository.findById(task.id);

    ASSERT_TRUE(stored_task.has_value());
    EXPECT_EQ(tasks::TaskStatus::kCompleted, stored_task->status);

    const tasks::TaskSchedulerMetrics metrics = scheduler->metrics();

    EXPECT_EQ(1U, metrics.scheduled_tasks);
    EXPECT_EQ(1U, metrics.completed_tasks);
    EXPECT_EQ(0U, metrics.failed_tasks);
}

//------------------------------------------------------------------------------

TEST_F(TaskSchedulerTest, CancelPreventsTaskExecution)
{
    net::io_context io_context;

    tasks::TaskRepository repository(":memory:");

    auto scheduler = std::make_shared<tasks::TaskScheduler>(
        io_context.get_executor(),
        repository);

    const tasks::TaskInput input = makeTaskInput(
        "cancelled task",
        tasks::currentUnixTimeMs() + 50);

    const tasks::Task task = repository.create(input);

    runAwaitable(io_context, scheduler->schedule(task));
    runAwaitable(io_context, scheduler->cancel(task.id));
    runFor(io_context, 500ms);

    const std::optional<tasks::Task> stored_task = repository.findById(task.id);

    ASSERT_TRUE(stored_task.has_value());
    EXPECT_EQ(tasks::TaskStatus::kScheduled, stored_task->status);

    const tasks::TaskSchedulerMetrics metrics = scheduler->metrics();

    EXPECT_EQ(1U, metrics.scheduled_tasks);
    EXPECT_EQ(1U, metrics.cancelled_tasks);
    EXPECT_EQ(0U, metrics.completed_tasks);
}

//------------------------------------------------------------------------------

TEST_F(TaskSchedulerTest, RescheduleIgnoresOldTimerCallback)
{
    net::io_context io_context;

    tasks::TaskRepository repository(":memory:");

    auto scheduler = std::make_shared<tasks::TaskScheduler>(
        io_context.get_executor(),
        repository);

    const tasks::TaskInput input = makeTaskInput(
        "rescheduled task",
        tasks::currentUnixTimeMs() + 50);

    const tasks::Task task = repository.create(input);

    runAwaitable(io_context, scheduler->schedule(task));

    const tasks::TaskInput updated_input = makeTaskInput(
        "rescheduled task",
        tasks::currentUnixTimeMs() + 600);

    const std::optional<tasks::Task> updated_task = repository.update(
        task.id,
        updated_input);

    ASSERT_TRUE(updated_task.has_value());

    runAwaitable(io_context, scheduler->schedule(*updated_task));
    runFor(io_context, 350ms);

    std::optional<tasks::Task> stored_task = repository.findById(task.id);

    ASSERT_TRUE(stored_task.has_value());
    EXPECT_EQ(tasks::TaskStatus::kScheduled, stored_task->status);

    runFor(io_context, 800ms);

    stored_task = repository.findById(task.id);

    ASSERT_TRUE(stored_task.has_value());
    EXPECT_EQ(tasks::TaskStatus::kCompleted, stored_task->status);

    const tasks::TaskSchedulerMetrics metrics = scheduler->metrics();

    EXPECT_EQ(2U, metrics.scheduled_tasks);
    EXPECT_EQ(1U, metrics.cancelled_tasks);
    EXPECT_EQ(1U, metrics.completed_tasks);
}

//------------------------------------------------------------------------------

TEST_F(TaskSchedulerTest, RestoreSchedulesStoredTasks)
{
    net::io_context io_context;

    tasks::TaskRepository repository(":memory:");

    auto scheduler = std::make_shared<tasks::TaskScheduler>(
        io_context.get_executor(),
        repository);

    const tasks::TaskInput input = makeTaskInput(
        "restored task",
        tasks::currentUnixTimeMs() + 50);

    const tasks::Task task = repository.create(input);

    runAwaitable(io_context, scheduler->restore());
    runFor(io_context, 700ms);

    const std::optional<tasks::Task> stored_task = repository.findById(task.id);

    ASSERT_TRUE(stored_task.has_value());
    EXPECT_EQ(tasks::TaskStatus::kCompleted, stored_task->status);

    const tasks::TaskSchedulerMetrics metrics = scheduler->metrics();

    EXPECT_EQ(1U, metrics.scheduled_tasks);
    EXPECT_EQ(1U, metrics.completed_tasks);
}

//------------------------------------------------------------------------------

} // namespace
