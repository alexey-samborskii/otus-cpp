#include "tasks/TaskService.hpp"

#include "tasks/TaskRepository.hpp"
#include "tasks/TaskScheduler.hpp"

#include <algorithm>
#include <cctype>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace net = boost::asio;

namespace tasks
{

//------------------------------------------------------------------------------

TaskNotFoundError::TaskNotFoundError(const std::string &message)
    : std::runtime_error(message)
{
}

//------------------------------------------------------------------------------

TaskService::TaskService(
    TaskRepository                &repository,
    std::shared_ptr<TaskScheduler> scheduler)
    : repository_(repository)
    , scheduler_(std::move(scheduler))
{
}

//------------------------------------------------------------------------------

net::awaitable<Task> TaskService::create(TaskInput input)
{
    validate(input);

    Task task = repository_.create(input);

    co_await scheduler_->schedule(task);

    co_return task;
}

//------------------------------------------------------------------------------

net::awaitable<Task> TaskService::findById(TaskId id) const
{
    const std::optional<Task> task = repository_.findById(id);

    if (!task.has_value())
    {
        throw TaskNotFoundError("Task was not found");
    }

    co_return *task;
}

//------------------------------------------------------------------------------

net::awaitable<std::vector<Task>> TaskService::findAll() const
{
    co_return repository_.findAll();
}

//------------------------------------------------------------------------------

net::awaitable<Task> TaskService::update(
    TaskId    id,
    TaskInput input)
{
    validate(input);

    co_await scheduler_->cancel(id);

    const std::optional<Task> task = repository_.update(id, input);

    if (!task.has_value())
    {
        throw TaskNotFoundError("Task was not found");
    }

    co_await scheduler_->schedule(*task);

    co_return *task;
}

//------------------------------------------------------------------------------

net::awaitable<void> TaskService::remove(TaskId id)
{
    co_await scheduler_->cancel(id);

    if (!repository_.remove(id))
    {
        throw TaskNotFoundError("Task was not found");
    }
}

//------------------------------------------------------------------------------

net::awaitable<Task> TaskService::runNow(TaskId id)
{
    const std::optional<Task> current_task = repository_.findById(id);

    if (!current_task.has_value())
    {
        throw TaskNotFoundError("Task was not found");
    }

    co_await scheduler_->cancel(id);

    TaskInput input;

    input.title           = current_task->title;
    input.description     = current_task->description;
    input.scheduled_at_ms = currentUnixTimeMs();

    const std::optional<Task> updated_task = repository_.update(
        id,
        input);

    if (!updated_task.has_value())
    {
        throw TaskNotFoundError("Task was not found");
    }

    co_await scheduler_->runNow(*updated_task);

    co_return *updated_task;
}

//------------------------------------------------------------------------------

void TaskService::validate(const TaskInput &input)
{
    const bool has_non_space_character = std::any_of(
        input.title.begin(),
        input.title.end(),
        [](unsigned char character) {
            return !std::isspace(character);
        });

    if (!has_non_space_character)
    {
        throw std::invalid_argument("Task title must not be empty");
    }

    if (input.title.size() > 200)
    {
        throw std::invalid_argument(
            "Task title must not exceed 200 characters");
    }

    if (input.description.size() > 5000)
    {
        throw std::invalid_argument(
            "Task description must not exceed 5000 characters");
    }

    if (input.scheduled_at_ms <= 0)
    {
        throw std::invalid_argument(
            "scheduledAt must contain Unix time in milliseconds");
    }
}

//------------------------------------------------------------------------------

} // namespace tasks
