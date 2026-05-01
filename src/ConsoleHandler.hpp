#pragma once

#include "ICommandBlockHandler.hpp"

namespace bulk
{

class ConsoleHandler : public ICommandBlockHandler
{
public:
    void handle(const CommandBlock& bulk) override;
};

} // namespace bulk