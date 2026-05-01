#pragma once

#include "CommandBlock.hpp"

namespace bulk
{

class ICommandBlockHandler
{
public:
    virtual ~ICommandBlockHandler()               = default;
    virtual void handle(const CommandBlock& bulk) = 0;
};

} // namespace bulk