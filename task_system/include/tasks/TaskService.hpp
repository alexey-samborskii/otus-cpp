#pragma once

#include "tasks/Task.hpp"

#include <boost/asio/awaitable.hpp>

#include <memory>
#include <optional>
#include <stdexcept>
#include <vector>

namespace tasks
{

class TaskRepository;
class TaskScheduler;

//------------------------------------------------------------------------------

class TaskNotFoundError : public std::runtime_error
{
public:
    explicit TaskNotFoundError(const std::string &message);
};

//------------------------------------------------------------------------------

class TaskService
{
public:
    TaskService(
        TaskRepository                &repository,
        std::shared_ptr<TaskScheduler> scheduler);

    auto create(TaskInput input) -> boost::asio::awaitable<Task>;
    auto findById(TaskId id) const -> boost::asio::awaitable<Task>;
    auto findAll() const -> boost::asio::awaitable<std::vector<Task>>;
    auto update(TaskId id, TaskInput input) -> boost::asio::awaitable<Task>;
    auto remove(TaskId id) -> boost::asio::awaitable<void>;
    auto runNow(TaskId id) -> boost::asio::awaitable<Task>;

private:
    static void validate(const TaskInput &input);

private:
    TaskRepository                &repository_;
    std::shared_ptr<TaskScheduler> scheduler_;
};

//------------------------------------------------------------------------------

} // namespace tasks
