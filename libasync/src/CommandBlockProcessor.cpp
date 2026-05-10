#include "CommandBlockProcessor.hpp"

#include <stdexcept>
#include <utility>

namespace bulk
{

//------------------------------------------------------------------------------

CommandBlockProcessor::CommandBlockProcessor(std::size_t           block_size,
                                             ICommandBlockHandler& handler,
                                             Clock                 clock)
    : block_size_(block_size)
    , handler_(handler)
    , clock_(std::move(clock))
{
    if (block_size_ == 0)
    {
        throw std::invalid_argument("block size must be greater than zero");
    }
}

//------------------------------------------------------------------------------

void CommandBlockProcessor::processLine(std::string line)
{
    if (line == "{")
    {
        if (!isDynamicBlockActive())
        {
            flush();
        }

        ++dynamic_depth_;
        return;
    }

    if (line == "}")
    {
        if (!isDynamicBlockActive())
        {
            return;
        }

        --dynamic_depth_;

        if (!isDynamicBlockActive())
        {
            flush();
        }

        return;
    }

    addCommand(std::move(line));

    if (!isDynamicBlockActive() && current_block_comand_.commands.size() == block_size_)
    {
        flush();
    }
}

//------------------------------------------------------------------------------

void CommandBlockProcessor::finish()
{
    if (isDynamicBlockActive())
    {
        clear();
        dynamic_depth_ = 0;
        return;
    }

    flush();
}

//------------------------------------------------------------------------------

bool CommandBlockProcessor::isDynamicBlockActive() const
{
    return dynamic_depth_ != 0;
}

//------------------------------------------------------------------------------

void CommandBlockProcessor::addCommand(std::string command)
{
    if (current_block_comand_.commands.empty())
    {
        current_block_comand_.timestamp = clock_();
    }

    current_block_comand_.commands.push_back(std::move(command));
}

//------------------------------------------------------------------------------

void CommandBlockProcessor::flush()
{
    if (current_block_comand_.commands.empty())
    {
        return;
    }

    handler_.handle(current_block_comand_);
    clear();
}

//------------------------------------------------------------------------------

void CommandBlockProcessor::clear()
{
    current_block_comand_ = CommandBlock{};
}

} // namespace bulk