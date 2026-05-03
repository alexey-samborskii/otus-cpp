#include "bayan_core/DuplicateFinder.hpp"

#include <utility>

namespace bayan
{

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

    const auto same_size_groups = groupBySize(files);

    for (const auto& same_size_group : same_size_groups)
    {
        const auto  file_size = same_size_group.first;
        const auto& group     = same_size_group.second;

        if (group.size() < 2)
        {
            continue;
        }

        auto duplicate_groups =
            findDuplicatesInSameSizeGroup(group, file_size, readers);

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

std::vector<FileGroup> DuplicateFinder::findDuplicatesInSameSizeGroup(
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
        std::vector<FileGroup> next_groups;

        for (const auto& group : groups)
        {
            if (group.size() < 2)
            {
                continue;
            }

            std::map<std::string, FileGroup> groups_by_hash;

            for (const auto file_index : group)
            {
                auto hash = readers[file_index].getBlockHash(block_index);
                if (hash)
                {
                    groups_by_hash[*hash].push_back(file_index);
                }
            }

            for (auto& same_hash_group : groups_by_hash)
            {
                if (same_hash_group.second.size() > 1)
                {
                    next_groups.push_back(std::move(same_hash_group.second));
                }
            }
        }

        groups = std::move(next_groups);

        if (groups.empty())
        {
            break;
        }
    }

    return groups;
}

} // namespace bayan