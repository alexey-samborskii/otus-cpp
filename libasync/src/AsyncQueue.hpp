#pragma once

#include <condition_variable>
#include <mutex>
#include <queue>
#include <utility>

namespace bulk
{

//------------------------------------------------------------------------------

template <typename T>
class AsyncQueue
{
public:
    AsyncQueue() = default;

    AsyncQueue(const AsyncQueue&)            = delete;
    AsyncQueue& operator=(const AsyncQueue&) = delete;

    void push(T value)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        if (closed_)
        {
            return;
        }

        queue_.push(std::move(value));

        lock.unlock();

        condition_.notify_one();
    }

    bool pop(T& value)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        condition_.wait(lock, [this] {
            return closed_ || !queue_.empty();
        });

        if (queue_.empty())
        {
            return false;
        }

        value = std::move(queue_.front());
        queue_.pop();

        return true;
    }

    void close()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        closed_ = true;
        lock.unlock();

        condition_.notify_all();
    }

private:
    std::mutex              mutex_;
    std::condition_variable condition_;
    std::queue<T>           queue_;
    bool                    closed_ = false;
};

//------------------------------------------------------------------------------

} // namespace bulk