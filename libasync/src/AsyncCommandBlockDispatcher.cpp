#include "AsyncCommandBlockDispatcher.hpp"

#include "ConsoleHandler.hpp"
#include "FileHandler.hpp"

#include <exception>
#include <iostream>
#include <mutex>

namespace bulk
{

//------------------------------------------------------------------------------

AsyncCommandBlockDispatcher::AsyncCommandBlockDispatcher()
    : log_thread_(&AsyncCommandBlockDispatcher::logWorker, this)
    , file_thread_1_(&AsyncCommandBlockDispatcher::fileWorker, this)
    , file_thread_2_(&AsyncCommandBlockDispatcher::fileWorker, this)
{
}

//------------------------------------------------------------------------------

AsyncCommandBlockDispatcher::~AsyncCommandBlockDispatcher()
{
    stop();
}

//------------------------------------------------------------------------------

void AsyncCommandBlockDispatcher::handle(const CommandBlock& block)
{
    std::lock_guard<std::mutex> lock(stop_mutex_);

    if (stopped_)
    {
        return;
    }

    log_queue_.push(block);
    file_queue_.push(block);
}

//------------------------------------------------------------------------------

void AsyncCommandBlockDispatcher::stop()
{
    {
        std::lock_guard<std::mutex> lock(stop_mutex_);

        if (stopped_)
        {
            return;
        }

        stopped_ = true;
    }

    log_queue_.close();
    file_queue_.close();

    if (log_thread_.joinable())
    {
        log_thread_.join();
    }

    if (file_thread_1_.joinable())
    {
        file_thread_1_.join();
    }

    if (file_thread_2_.joinable())
    {
        file_thread_2_.join();
    }
}

//------------------------------------------------------------------------------

void AsyncCommandBlockDispatcher::logWorker()
{
    ConsoleHandler     handler;
    bulk::CommandBlock block;
    while (log_queue_.pop(block))
    {
        handler.handle(block);
    }
}

//------------------------------------------------------------------------------

void AsyncCommandBlockDispatcher::fileWorker()
{
    FileHandler        handler;
    bulk::CommandBlock block;
    while (file_queue_.pop(block))
    {
        handler.handle(block);
    }
}

//------------------------------------------------------------------------------

} // namespace bulk