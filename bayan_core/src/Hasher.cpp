#include "bayan_core/Hasher.hpp"

#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>

#include <iomanip>
#include <sstream>

namespace bayan
{

//------------------------------------------------------------------------------

Hasher::Hasher(HashAlgorithm algorithm)
    : algorithm_(algorithm)
{
}

//------------------------------------------------------------------------------

std::string Hasher::hashBlock(const std::vector<char>& block) const
{
    switch (algorithm_)
    {
    case HashAlgorithm::Md5:
        return md5Hash(block.data(), block.size());

    case HashAlgorithm::Crc32:
        return crc32Hash(block.data(), block.size());
    }

    return crc32Hash(block.data(), block.size());
}

//------------------------------------------------------------------------------

std::string Hasher::crc32Hash(const char* data, std::size_t size) const
{
    boost::crc_32_type crc;

    crc.process_bytes(data, size);

    return std::to_string(crc.checksum());
}

//------------------------------------------------------------------------------

std::string Hasher::md5Hash(const char* data, std::size_t size) const
{
    boost::uuids::detail::md5              md5;
    boost::uuids::detail::md5::digest_type digest;

    md5.process_bytes(data, size);
    md5.get_digest(digest);

    std::ostringstream stream;

    for (const auto value : digest)
    {
        stream << std::hex
               << std::setw(8)
               << std::setfill('0')
               << value;
    }

    return stream.str();
}

//------------------------------------------------------------------------------

} // namespace bayan