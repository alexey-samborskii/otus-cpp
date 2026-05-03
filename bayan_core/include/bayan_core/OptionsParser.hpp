#pragma once

#include "Config.hpp"

namespace bayan
{

//------------------------------------------------------------------------------

class OptionsParser
{
public:
    Config parse(int argc, char* argv[]) const;
};

//------------------------------------------------------------------------------

} // namespace bayan