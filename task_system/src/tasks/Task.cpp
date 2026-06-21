#include "tasks/Task.hpp"

#include <chrono>
#include <stdexcept>

namespace tasks
{
    
//------------------------------------------------------------------------------

std::string toString(TaskStatus status)
{
    switch (status)
    {
        case TaskStatus::kScheduled:
            return "scheduled";

        case TaskStatus::kRunning:
            return "running";

        case TaskStatus::kCompleted:
            return "completed";

        case TaskStatus::kFailed:
            return "failed";
    }

    throw std::invalid_argument("Unknown task status");
}

//------------------------------------------------------------------------------

TaskStatus taskStatusFromString(const std::string &value)
{
    if (value == "scheduled")
    {
        return TaskStatus::kScheduled;
    }

    if (value == "running")
    {
        return TaskStatus::kRunning;
    }

    if (value == "completed")
    {
        return TaskStatus::kCompleted;
    }

    if (value == "failed")
    {
        return TaskStatus::kFailed;
    }

    throw std::invalid_argument("Unknown task status: " + value);
}

//------------------------------------------------------------------------------

std::int64_t currentUnixTimeMs()
{
    const auto now = std::chrono::system_clock::now();

    return std::chrono::duration_cast<std::chrono::milliseconds>(
               now.time_since_epoch())
        .count();
}

//------------------------------------------------------------------------------

} // namespace tasks
