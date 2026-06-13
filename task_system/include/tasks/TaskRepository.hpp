#pragma once

#include "tasks/Task.hpp"

#include <filesystem>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

struct sqlite3;

namespace tasks
{

class TaskRepository
{
public:
    explicit TaskRepository(
        const std::filesystem::path &database_path);

    ~TaskRepository();

    TaskRepository(const TaskRepository &) = delete;
    TaskRepository &operator=(const TaskRepository &) = delete;

    Task create(const TaskInput &input);

    std::optional<Task> findById(TaskId id) const;

    std::vector<Task> findAll() const;

    std::vector<Task> findScheduled() const;

    std::optional<Task> update(
        TaskId          id,
        const TaskInput &input);

    bool remove(TaskId id);

    bool claimForExecution(TaskId id);

    void setStatus(
        TaskId                     id,
        TaskStatus                 status,
        std::optional<std::string> error_message = std::nullopt);

private:
    void createSchema();
    void execute(const std::string &sql) const;

private:
    sqlite3            *database_ = nullptr;
    mutable std::mutex  mutex_;
};

} // namespace tasks
