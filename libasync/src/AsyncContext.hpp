#pragma once

#include "AsyncQueue.hpp"
#include "CommandBlock.hpp"
#include "CommandBlockProcessor.hpp"
#include "ICommandBlockHandler.hpp"

#include <cstddef>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace bulk
{

//------------------------------------------------------------------------------

class AsyncContext
{
public:
    explicit AsyncContext(std::size_t block_size);
    ~AsyncContext();

    AsyncContext(const AsyncContext&) = delete;
    AsyncContext& operator=(const AsyncContext&) = delete;

    void receive(const char* data, std::size_t size);
    void disconnect();

private:
    class AsyncDispatcher : public ICommandBlockHandler
    {
    public:
        AsyncDispatcher(AsyncQueue<CommandBlock>& log_queue,
                        AsyncQueue<CommandBlock>& file_queue);

        void handle(const CommandBlock& block) override;

    private:
        AsyncQueue<CommandBlock>& log_queue_;
        AsyncQueue<CommandBlock>& file_queue_;
    };

    void processLine(std::string line);

    void logWorker();
    void fileWorker();

private:
    AsyncQueue<CommandBlock> log_queue_;
    AsyncQueue<CommandBlock> file_queue_;

    AsyncDispatcher      dispatcher_;
    CommandBlockProcessor processor_;

    std::thread log_thread_;
    std::thread file_thread_1_;
    std::thread file_thread_2_;

    std::mutex  receive_mutex_;
    std::string input_buffer_;

    bool disconnected_ = false;
};

//------------------------------------------------------------------------------

} // namespace bulk