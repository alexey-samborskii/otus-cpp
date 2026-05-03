#include "bayan_core/OutputPrinter.hpp"

#include <algorithm>
#include <iostream>

namespace bayan
{

//------------------------------------------------------------------------------

void OutputPrinter::print(const std::vector<FileInfo>& files,
                          std::vector<FileGroup>       groups) const
{
    for (auto& group : groups)
    {
        std::sort(group.begin(),
                  group.end(),
                  [&files](FileIndex lhs, FileIndex rhs) {
                      return files[lhs].path.string() <
                             files[rhs].path.string();
                  });
    }

    std::sort(groups.begin(),
              groups.end(),
              [&files](const FileGroup& lhs, const FileGroup& rhs) {
                  return files[lhs.front()].path.string() <
                         files[rhs.front()].path.string();
              });

    for (std::size_t group_index = 0; group_index < groups.size(); ++group_index)
    {
        for (const auto file_index : groups[group_index])
        {
            std::cout << files[file_index].path.string() << '\n';
        }

        if (group_index + 1 < groups.size())
        {
            std::cout << '\n';
        }
    }
}

//------------------------------------------------------------------------------

} // namespace bayan