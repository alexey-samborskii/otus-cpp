#pragma once

#include <boost/filesystem.hpp>

#include <cstdint>
#include <string>
#include <vector>

namespace bayan
{

enum class HashAlgorithm
{
    Crc32,
    Md5
};

struct Config
{
    std::vector<boost::filesystem::path> scan_dirs;
    std::vector<boost::filesystem::path> exclude_dirs;
    std::vector<std::string>             masks;

    std::uintmax_t min_size   = 1;
    std::size_t    block_size = 4096;
    int            depth      = 0;

    HashAlgorithm hash_algorithm = HashAlgorithm::Crc32;
};

} // namespace bayan