#pragma once

#include <ctime>
#include <string>
#include <vector>

namespace bulk
{

struct CommandBlock
{
    std::time_t              timestamp = 0;
    std::vector<std::string> commands;
};

} // namespace bulk