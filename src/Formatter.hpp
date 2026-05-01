#pragma once

#include "CommandBlock.hpp"

#include <iosfwd>
#include <string>

namespace bulk
{

class Formatter
{
public:
    static std::string format(const CommandBlock& bulk);
    static void write(std::ostream& output, const CommandBlock& bulk);
};

} // namespace bulk