#include "bayan_core/DuplicateFinder.hpp"

#include <map>
#include <string>
#include <utility>
#include <vector>

namespace bayan
{

namespace
{

//------------------------------------------------------------------------------

std::map<std::string, FileGroup> groupFilesByBlockHash(
    const FileGroup&          group,
    std::size_t               block_index,
    std::vector<BlockReader>& readers)
{
    std::map<std::string, FileGroup> groups_by_hash;

    for (const auto file_index : group)
    {
        auto hash = readers[file_index].getBlockHash(block_index);
        if (hash)
        {
            groups_by_hash[*hash].push_back(file_index);
        }
    }

    return groups_by_hash;
}

//------------------------------------------------------------------------------

void appendDuplicateGroups(std::map<std::string, FileGroup>& groups_by_hash,
                           std::vector<FileGroup>&           result)
{
    for (auto& same_hash_group : groups_by_hash)
    {
        if (same_hash_group.second.size() > 1)
        {
            result.push_back(std::move(same_hash_group.second));
        }
    }
}

//------------------------------------------------------------------------------

void appendGroupsWithSameBlockHash(const FileGroup&          group,
                                   std::size_t               block_index,
                                   std::vector<BlockReader>& readers,
                                   std::vector<FileGroup>&   result)
{
    if (group.size() < 2)
    {
        return;
    }

    auto groups_by_hash = groupFilesByBlockHash(group, block_index, readers);

    appendDuplicateGroups(groups_by_hash, result);
}

//------------------------------------------------------------------------------

std::vector<FileGroup> splitGroupsByBlockHash(
    const std::vector<FileGroup>& groups,
    std::size_t                   block_index,
    std::vector<BlockReader>&     readers)
{
    std::vector<FileGroup> result;

    for (const auto& group : groups)
    {
        appendGroupsWithSameBlockHash(group, block_index, readers, result);
    }

    return result;
}

} // namespace

//------------------------------------------------------------------------------

DuplicateFinder::DuplicateFinder(Config config)
    : config_(std::move(config))
{
}

//------------------------------------------------------------------------------

std::vector<FileGroup> DuplicateFinder::find(
    const std::vector<FileInfo>& files) const
{
    std::vector<BlockReader> readers;
    readers.reserve(files.size());

    for (const auto& file : files)
    {
        readers.emplace_back(file,
                             config_.block_size,
                             Hasher(config_.hash_algorithm));
    }

    std::vector<FileGroup> result;

    const auto groups_by_size = groupBySize(files);

    for (const auto& [file_size, group] : groups_by_size)
    {
        if (group.size() < 2)
        {
            continue;
        }

        auto duplicate_groups = findDuplicatesByContent(group,
                                                        file_size,
                                                        readers);

        for (auto& duplicate_group : duplicate_groups)
        {
            result.push_back(std::move(duplicate_group));
        }
    }

    return result;
}

//------------------------------------------------------------------------------

std::map<std::uintmax_t, FileGroup> DuplicateFinder::groupBySize(
    const std::vector<FileInfo>& files) const
{
    std::map<std::uintmax_t, FileGroup> result;

    for (FileIndex index = 0; index < files.size(); ++index)
    {
        result[files[index].size].push_back(index);
    }

    return result;
}

//------------------------------------------------------------------------------

std::vector<FileGroup> DuplicateFinder::findDuplicatesByContent(
    const FileGroup&          source_group,
    std::uintmax_t            file_size,
    std::vector<BlockReader>& readers) const
{
    const auto total_blocks =
        static_cast<std::size_t>((file_size + config_.block_size - 1) /
                                 config_.block_size);

    std::vector<FileGroup> groups;
    groups.push_back(source_group);

    for (std::size_t block_index = 0; block_index < total_blocks; ++block_index)
    {
        groups = splitGroupsByBlockHash(groups, block_index, readers);

        if (groups.empty())
        {
            break;
        }
    }

    return groups;
}

//------------------------------------------------------------------------------

} // namespace bayan