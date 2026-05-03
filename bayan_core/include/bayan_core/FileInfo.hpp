#pragma once

#include <boost/filesystem.hpp>

#include <cstdint>
#include <vector>

namespace bayan
{

struct FileInfo
{
    boost::filesystem::path path;
    std::uintmax_t          size = 0;
};

using FileIndex = std::size_t;
using FileGroup = std::vector<FileIndex>;

} // namespace bayan