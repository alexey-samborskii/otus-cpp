#include "bayan_core/ArgumentsParser.hpp"

#include <boost/program_options.hpp>

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace bayan
{

namespace
{

namespace po = boost::program_options;

//------------------------------------------------------------------------------

struct RawOptions
{
    Config                   config;
    std::vector<std::string> scan_dirs;
    std::vector<std::string> exclude_dirs;
    std::string              hash_algorithm;
};

//------------------------------------------------------------------------------

HashAlgorithm parseHashAlgorithm(const std::string& value)
{
    if (value == "crc32")
    {
        return HashAlgorithm::Crc32;
    }

    if (value == "md5")
    {
        return HashAlgorithm::Md5;
    }

    throw std::runtime_error("unsupported hash algorithm: " + value);
}

//------------------------------------------------------------------------------

template <typename PathList>
void addPathList(PathList& result, const std::vector<std::string>& values)
{
    for (const auto& value : values)
    {
        result.emplace_back(value);
    }
}

//------------------------------------------------------------------------------

void addOptions(po::options_description& options, RawOptions& raw_options)
{
    auto add_option = options.add_options();

    add_option("help,h",
               "show help");

    add_option("include,I",
               po::value<std::vector<std::string>>(&raw_options.scan_dirs)
                   ->multitoken()
                   ->composing(),
               "directories for scanning");

    add_option("exclude,E",
               po::value<std::vector<std::string>>(&raw_options.exclude_dirs)
                   ->multitoken()
                   ->composing(),
               "directories to exclude from scanning");

    add_option("depth,d",
               po::value<int>(&raw_options.config.depth)->default_value(0),
               "recursive scanning depth, 0 means only selected directory");

    add_option("min-size,m",
               po::value<std::uintmax_t>(&raw_options.config.min_size)
                   ->default_value(1),
               "minimal file size, files with size <= value are ignored");

    add_option("mask,M",
               po::value<std::vector<std::string>>(
                   &raw_options.config.masks)
                   ->multitoken()
                   ->composing(),
               "file masks, for example *.txt *.cpp");

    add_option("block-size,S",
               po::value<std::size_t>(&raw_options.config.block_size)
                   ->default_value(4096),
               "block size for reading files");

    add_option("hash,H",
               po::value<std::string>(&raw_options.hash_algorithm)
                   ->default_value("crc32"),
               "hash algorithm: crc32 or md5");
}

//------------------------------------------------------------------------------

void validateOptions(const RawOptions& raw_options)
{
    if (raw_options.scan_dirs.empty())
    {
        throw std::runtime_error("at least one include directory is required");
    }

    if (raw_options.config.block_size == 0)
    {
        throw std::runtime_error("block size must be greater than zero");
    }
}

//------------------------------------------------------------------------------

Config makeConfig(const RawOptions& raw_options)
{
    validateOptions(raw_options);

    Config config = raw_options.config;

    config.hash_algorithm = parseHashAlgorithm(raw_options.hash_algorithm);

    addPathList(config.scan_dirs, raw_options.scan_dirs);
    addPathList(config.exclude_dirs, raw_options.exclude_dirs);

    return config;
}

} // namespace

//------------------------------------------------------------------------------

Config ArgumentsParser::parse(int argc, char* argv[]) const
{
    RawOptions raw_options;

    po::options_description options("Allowed options");

    addOptions(options, raw_options);

    po::variables_map variables;

    po::store(po::parse_command_line(argc, argv, options), variables);

    if (variables.count("help") != 0)
    {
        std::cout << options << '\n';
        std::exit(EXIT_SUCCESS);
    }

    po::notify(variables);

    return makeConfig(raw_options);
}

//------------------------------------------------------------------------------

} // namespace bayan