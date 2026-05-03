#include "bayan_core/OptionsParser.hpp"

#include <boost/program_options.hpp>

#include <cstdlib>
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

} // namespace

//------------------------------------------------------------------------------

Config OptionsParser::parse(int argc, char* argv[]) const
{
    Config config;

    std::vector<std::string> scan_dirs;
    std::vector<std::string> exclude_dirs;
    std::string              hash_algorithm;

    po::options_description options("Allowed options");

    options.add_options()("help,h",
                          "show help")

        ("include,I",
         po::value<std::vector<std::string>>(&scan_dirs)
             ->multitoken()
             ->composing(),
         "directories for scanning")

            ("exclude,E",
             po::value<std::vector<std::string>>(&exclude_dirs)
                 ->multitoken()
                 ->composing(),
             "directories to exclude from scanning")

                ("depth,d",
                 po::value<int>(&config.depth)->default_value(0),
                 "recursive scanning depth, 0 means only selected directory")

                    ("min-size,m",
                     po::value<std::uintmax_t>(&config.min_size)->default_value(1),
                     "minimal file size, files with size <= value are ignored")

                        ("mask,M",
                         po::value<std::vector<std::string>>(&config.masks)
                             ->multitoken()
                             ->composing(),
                         "file masks, for example *.txt *.cpp")

                            ("block-size,S",
                             po::value<std::size_t>(&config.block_size)->default_value(4096),
                             "block size for reading files")

                                ("hash,H",
                                 po::value<std::string>(&hash_algorithm)->default_value("crc32"),
                                 "hash algorithm: crc32 or md5");

    po::variables_map variables;

    po::store(po::parse_command_line(argc, argv, options), variables);

    if (variables.count("help") != 0)
    {
        std::cout << options << '\n';
        std::exit(0);
    }

    po::notify(variables);

    if (scan_dirs.empty())
    {
        throw std::runtime_error("at least one include directory is required");
    }

    if (config.block_size == 0)
    {
        throw std::runtime_error("block size must be greater than zero");
    }

    config.hash_algorithm = parseHashAlgorithm(hash_algorithm);

    for (const auto& directory : scan_dirs)
    {
        config.scan_dirs.emplace_back(directory);
    }

    for (const auto& directory : exclude_dirs)
    {
        config.exclude_dirs.emplace_back(directory);
    }

    return config;
}

//------------------------------------------------------------------------------

} // namespace bayan