#pragma once

#include "Config.hpp"

#include <cstddef>
#include <string>
#include <vector>

namespace bayan
{

//------------------------------------------------------------------------------

class Hasher
{
public:
    explicit Hasher(HashAlgorithm algorithm);

    std::string hashBlock(const std::vector<char>& block) const;

private:
    std::string crc32Hash(const char* data, std::size_t size) const;
    std::string md5Hash(const char* data, std::size_t size) const;

private:
    HashAlgorithm algorithm_;
};

//------------------------------------------------------------------------------

} // namespace bayan