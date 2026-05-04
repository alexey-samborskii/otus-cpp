#pragma once

#include "BlockReader.hpp"
#include "Config.hpp"
#include "FileInfo.hpp"

#include <map>
#include <vector>

namespace bayan
{

//------------------------------------------------------------------------------

class DuplicateFinder
{
public:
    explicit DuplicateFinder(Config config);

    std::vector<FileGroup> find(const std::vector<FileInfo>& files) const;

private:
    std::map<std::uintmax_t, FileGroup> groupBySize(
        const std::vector<FileInfo>& files) const;

    std::vector<FileGroup> findDuplicatesByContent(
        const FileGroup&          source_group,
        std::uintmax_t            file_size,
        std::vector<BlockReader>& readers) const;

private:
    Config config_;
};

//------------------------------------------------------------------------------

} // namespace bayan