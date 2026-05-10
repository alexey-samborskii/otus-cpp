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
        std::unique_lock<std::mutex> lock(mux_);

        if (closed_)
        {
            return;
        }

        queue_.push(std::move(value));

        lock.unlock();

        cv_.notify_one();
    }

    bool pop(T& value)
    {
        std::unique_lock<std::mutex> lock(mux_);

        cv_.wait(lock, [this] {
            return closed_ || !queue_.empty();
        });

        if (closed_ && queue_.empty())
        {
            return false;
        }

        value = std::move(queue_.front());
        queue_.pop();

        return true;
    }

    void close()
    {
        std::unique_lock<std::mutex> lock(mux_);
        closed_ = true;
        lock.unlock();
        cv_.notify_all();
    }

private:
    std::mutex              mux_;
    std::condition_variable cv_;
    std::queue<T>           queue_;
    bool                    closed_ = false;
};

//------------------------------------------------------------------------------

} // namespace bulk