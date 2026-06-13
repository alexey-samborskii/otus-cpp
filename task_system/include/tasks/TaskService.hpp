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

class TaskNotFoundError : public std::runtime_error
{
public:
    explicit TaskNotFoundError(const std::string &message);
};

class TaskService
{
public:
    TaskService(
        TaskRepository                &repository,
        std::shared_ptr<TaskScheduler> scheduler);

    boost::asio::awaitable<Task> create(TaskInput input);

    boost::asio::awaitable<Task> findById(TaskId id) const;

    boost::asio::awaitable<std::vector<Task>> findAll() const;

    boost::asio::awaitable<Task> update(
        TaskId    id,
        TaskInput input);

    boost::asio::awaitable<void> remove(TaskId id);

    boost::asio::awaitable<Task> runNow(TaskId id);

private:
    static void validate(const TaskInput &input);

private:
    TaskRepository                &repository_;
    std::shared_ptr<TaskScheduler> scheduler_;
};

} // namespace tasks
