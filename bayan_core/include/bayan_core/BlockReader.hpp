#pragma once

#include "FileInfo.hpp"
#include "Hasher.hpp"

#include <cstddef>
#include <fstream>
#include <optional>
#include <string>
#include <vector>

namespace bayan
{

//------------------------------------------------------------------------------

class BlockReader
{
public:
    BlockReader(FileInfo    file,
                std::size_t block_size,
                Hasher      hasher);

    std::optional<std::string> getBlockHash(std::size_t block_index);

private:
    bool readNextBlock();

private:
    FileInfo file_;

    std::size_t block_size_   = 0;
    std::size_t total_blocks_ = 0;

    Hasher hasher_;

    std::ifstream            stream_;
    std::vector<std::string> hashes_;
};

//------------------------------------------------------------------------------

} // namespace bayan