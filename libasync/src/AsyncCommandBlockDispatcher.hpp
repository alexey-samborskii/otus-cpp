#pragma once

#include "AsyncQueue.hpp"
#include "CommandBlock.hpp"
#include "ICommandBlockHandler.hpp"

#include <mutex>
#include <thread>

namespace bulk
{

//------------------------------------------------------------------------------

class AsyncCommandBlockDispatcher : public ICommandBlockHandler
{
public:
    AsyncCommandBlockDispatcher();
    ~AsyncCommandBlockDispatcher();

    AsyncCommandBlockDispatcher(const AsyncCommandBlockDispatcher&) = delete;
    AsyncCommandBlockDispatcher& operator=(const AsyncCommandBlockDispatcher&) = delete;

    void handle(const CommandBlock& block) override;

    void stop();

private:
    void logWorker();
    void fileWorker();

private:
    AsyncQueue<CommandBlock> log_queue_;
    std::thread log_thread_;
    
    AsyncQueue<CommandBlock> file_queue_;    
    std::thread file_thread_1_;
    std::thread file_thread_2_;

    std::mutex stop_mutex_;
    bool       stopped_ = false;
};

//------------------------------------------------------------------------------

} // namespace bulk