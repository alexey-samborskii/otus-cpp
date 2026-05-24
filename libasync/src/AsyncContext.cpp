#include "AsyncContext.hpp"

#include <utility>

namespace bulk
{

//------------------------------------------------------------------------------

AsyncContext::AsyncContext(std::size_t block_size)
    : dispatcher_()
    , processor_(block_size, dispatcher_)
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
        if (data[i] == '\n')
        {
            processor_.processLine(std::move(input_buffer_));
            input_buffer_.clear();
        }
        else
        {
            input_buffer_.push_back(data[i]);
        }
    }
}

//------------------------------------------------------------------------------

void AsyncContext::flush()
{
    processor_.flush();
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
            processor_.processLine(std::move(input_buffer_));
            input_buffer_.clear();
        }

        processor_.finish();

        disconnected_ = true;
    }

    dispatcher_.stop();
}

//------------------------------------------------------------------------------

} // namespace bulk