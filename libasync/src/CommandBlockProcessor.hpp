#pragma once

#include "CommandBlock.hpp"
#include "ICommandBlockHandler.hpp"

#include <cstddef>
#include <ctime>
#include <functional>
#include <string>

namespace bulk
{

class CommandBlockProcessor
{
public:
    using Clock = std::function<std::time_t()>;

    CommandBlockProcessor(std::size_t block_size,
                  ICommandBlockHandler& handler,
                  Clock clock = [] {
                      return std::time(nullptr);
                  });

    void processLine(std::string line);
    void finish();
    void flush();
    
private:
    bool isDynamicBlockActive() const;

    void addCommand(std::string command);

    void clear();

private:
    std::size_t   block_size_;
    ICommandBlockHandler& handler_;
    Clock         clock_;

    std::size_t dynamic_depth_ = 0;
    CommandBlock        current_block_comand_;
};

} // namespace bulk