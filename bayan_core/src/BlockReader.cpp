#include "bayan_core/BlockReader.hpp"

#include <iostream>
#include <utility>

namespace bayan
{

//------------------------------------------------------------------------------

BlockReader::BlockReader(FileInfo    file,
                         std::size_t block_size,
                         Hasher      hasher)
    : file_(std::move(file))
    , block_size_(block_size)
    , hasher_(std::move(hasher))
{
    total_blocks_ =
        static_cast<std::size_t>((file_.size + block_size_ - 1) / block_size_);
}

//------------------------------------------------------------------------------

std::optional<std::string> BlockReader::getBlockHash(std::size_t block_index)
{
    while (hashes_.size() <= block_index)
    {
        if (!readNextBlock())
        {
            return std::nullopt;
        }
    }

    return hashes_[block_index];
}

//------------------------------------------------------------------------------

bool BlockReader::readNextBlock()
{
    if (hashes_.size() >= total_blocks_)
    {
        return false;
    }

    if (!stream_.is_open())
    {
        stream_.open(file_.path.string(), std::ios::binary);
        if (!stream_)
        {
            std::cerr << "Cannot open file: "
                      << file_.path.string()
                      << '\n';
            return false;
        }
    }

    std::vector<char> block(block_size_, 0);

    stream_.read(block.data(), static_cast<std::streamsize>(block.size()));

    const auto bytes_read    = stream_.gcount();
    const bool is_last_block = hashes_.size() + 1 == total_blocks_;

    if (stream_.bad())
    {
        std::cerr << "Cannot read file: "
                  << file_.path.string()
                  << '\n';
        return false;
    }

    if (!is_last_block &&
        static_cast<std::size_t>(bytes_read) != block_size_)
    {
        std::cerr << "File was changed while reading: "
                  << file_.path.string()
                  << '\n';
        return false;
    }

    hashes_.push_back(hasher_.hashBlock(block));

    return true;
}

//------------------------------------------------------------------------------

} // namespace bayan