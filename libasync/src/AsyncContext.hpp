#pragma once

#include "AsyncCommandBlockDispatcher.hpp"
#include "CommandBlockProcessor.hpp"

#include <cstddef>
#include <mutex>
#include <string>

namespace bulk
{

//------------------------------------------------------------------------------

class AsyncContext
{
public:
    explicit AsyncContext(std::size_t block_size);
    ~AsyncContext();

    AsyncContext(const AsyncContext&)            = delete;
    AsyncContext& operator=(const AsyncContext&) = delete;

    void receive(const char* data, std::size_t size);
    void disconnect();

private:
    AsyncCommandBlockDispatcher dispatcher_;
    CommandBlockProcessor       processor_;

    std::mutex  receive_mutex_;
    std::string input_buffer_;
    bool        disconnected_ = false;
};

//------------------------------------------------------------------------------

} // namespace bulk