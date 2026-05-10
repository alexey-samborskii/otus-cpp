#include "AsyncContext.hpp"

#include "ConsoleHandler.hpp"
#include "FileHandler.hpp"

#include <utility>

namespace bulk
{

//------------------------------------------------------------------------------

AsyncContext::AsyncDispatcher::AsyncDispatcher(
    AsyncQueue<CommandBlock>& log_queue,
    AsyncQueue<CommandBlock>& file_queue)
    : log_queue_(log_queue)
    , file_queue_(file_queue)
{
}

//------------------------------------------------------------------------------

void AsyncContext::AsyncDispatcher::handle(const CommandBlock& block)
{
    log_queue_.push(block);
    file_queue_.push(block);
}

//------------------------------------------------------------------------------

AsyncContext::AsyncContext(std::size_t block_size)
    : dispatcher_(log_queue_, file_queue_)
    , processor_(block_size, dispatcher_)
    , log_thread_(&AsyncContext::logWorker, this)
    , file_thread_1_(&AsyncContext::fileWorker, this)
    , file_thread_2_(&AsyncContext::fileWorker, this)
{
}

//------------------------------------------------------------------------------

AsyncContext::~AsyncContext()
{
    disconnect();
}

//------------------------------------------------------------------------------

void AsyncContext::receive(const char* data, std::size_t size)
{
    if (data == nullptr || size == 0)
    {
        return;
    }

    std::lock_guard<std::mutex> lock(receive_mutex_);

    if (disconnected_)
    {
        return;
    }

    for (std::size_t i = 0; i < size; ++i)
    {
        const char ch = data[i];

        if (ch == '\n')
        {
            processLine(std::move(input_buffer_));
            input_buffer_.clear();
        }
        else
        {
            input_buffer_.push_back(ch);
        }
    }
}

//------------------------------------------------------------------------------

void AsyncContext::disconnect()
{
    {
        std::lock_guard<std::mutex> lock(receive_mutex_);

        if (disconnected_)
        {
            return;
        }

        if (!input_buffer_.empty())
        {
            processLine(std::move(input_buffer_));
            input_buffer_.clear();
        }

        processor_.finish();
        disconnected_ = true;
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

void AsyncContext::processLine(std::string line)
{
    processor_.processLine(std::move(line));
}

//------------------------------------------------------------------------------

void AsyncContext::logWorker()
{
    ConsoleHandler handler;

    CommandBlock block;
    while (log_queue_.pop(block))
    {
        handler.handle(block);
    }
}

//------------------------------------------------------------------------------

void AsyncContext::fileWorker()
{
    FileHandler handler;

    CommandBlock block;
    while (file_queue_.pop(block))
    {
        handler.handle(block);
    }
}

//------------------------------------------------------------------------------

} // namespace bulk