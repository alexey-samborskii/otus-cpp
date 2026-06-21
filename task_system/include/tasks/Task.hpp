#pragma once

#include <cstdint>
#include <optional>
#include <string>

namespace tasks
{

using TaskId = std::int64_t;

//------------------------------------------------------------------------------

enum class TaskStatus
{
    kScheduled = 0,
    kRunning,
    kCompleted,
    kFailed
};

//------------------------------------------------------------------------------
struct Task
{
    TaskId                     id = 0;
    std::string                title;
    std::string                description;
    std::int64_t               scheduled_at_ms = 0;
    TaskStatus                 status = TaskStatus::kScheduled;
    std::optional<std::string> error_message;
    std::int64_t               created_at_ms = 0;
    std::int64_t               updated_at_ms = 0;
};

//------------------------------------------------------------------------------

struct TaskInput
{
    std::string  title;
    std::string  description;
    std::int64_t scheduled_at_ms = 0;
};

//------------------------------------------------------------------------------

std::string toString(TaskStatus status);
TaskStatus taskStatusFromString(const std::string &value);
std::int64_t currentUnixTimeMs();

//------------------------------------------------------------------------------

} // namespace tasks
