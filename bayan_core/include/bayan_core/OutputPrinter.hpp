#pragma once

#include "FileInfo.hpp"

#include <vector>

namespace bayan
{

//------------------------------------------------------------------------------

class OutputPrinter
{
public:
    void print(const std::vector<FileInfo>& files,
               std::vector<FileGroup>       groups) const;
};

//------------------------------------------------------------------------------

} // namespace bayan