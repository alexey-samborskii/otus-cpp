#pragma once

#include "Config.hpp"

namespace bayan
{

//------------------------------------------------------------------------------

class ArgumentsParser
{
public:
    Config parse(int argc, char* argv[]) const;
};

//------------------------------------------------------------------------------

} // namespace bayan