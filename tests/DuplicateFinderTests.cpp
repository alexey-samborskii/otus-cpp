#include "bayan_core/Config.hpp"
#include "bayan_core/DuplicateFinder.hpp"
#include "bayan_core/FileScanner.hpp"

#include <boost/filesystem.hpp>

#include <gtest/gtest.h>

#include <algorithm>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace
{

namespace fs = boost::filesystem;

//------------------------------------------------------------------------------

class TemporaryDirectory
{
public:
    TemporaryDirectory()
    {
        path_ = fs::temp_directory_path() /
                fs::unique_path("bayan-tests-%%%%-%%%%-%%%%");

        fs::create_directories(path_);
    }

    ~TemporaryDirectory()
    {
        boost::system::error_code ec;
        fs::remove_all(path_, ec);
    }

    const fs::path& path() const
    {
        return path_;
    }

private:
    fs::path path_;
};

//------------------------------------------------------------------------------

void writeFile(const fs::path& path, const std::string& content)
{
    fs::create_directories(path.parent_path());

    std::ofstream file(path.string(), std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("cannot create file: " + path.string());
    }

    file.write(content.data(), static_cast<std::streamsize>(content.size()));

    if (!file)
    {
        throw std::runtime_error("cannot write file: " + path.string());
    }
}

//------------------------------------------------------------------------------

std::string canonicalString(const fs::path& path)
{
    return fs::canonical(path).string();
}

//------------------------------------------------------------------------------

bayan::Config makeConfig(const fs::path& directory)
{
    bayan::Config config;

    config.scan_dirs.push_back(directory);
    config.min_size       = 0;
    config.block_size     = 5;
    config.depth          = 3;
    config.hash_algorithm = bayan::HashAlgorithm::Crc32;

    return config;
}

//------------------------------------------------------------------------------

std::vector<std::set<std::string>> toPathGroups(
    const std::vector<bayan::FileInfo>&  files,
    const std::vector<bayan::FileGroup>& groups)
{
    std::vector<std::set<std::string>> result;

    for (const auto& group : groups)
    {
        std::set<std::string> paths;

        for (const auto file_index : group)
        {
            paths.insert(files[file_index].path.string());
        }

        result.push_back(std::move(paths));
    }

    return result;
}

//------------------------------------------------------------------------------

bool containsGroup(const std::vector<std::set<std::string>>& groups,
                   const std::set<std::string>&              expected)
{
    return std::find(groups.begin(), groups.end(), expected) != groups.end();
}

} // namespace

//------------------------------------------------------------------------------

TEST(DuplicateFinderTests, FindsFilesWithIdenticalContent)
{
    TemporaryDirectory temp;

    const auto first_file  = temp.path() / "first.txt";
    const auto second_file = temp.path() / "nested" / "second.txt";
    const auto other_file  = temp.path() / "other.txt";

    writeFile(first_file, "Hello duplicate file");
    writeFile(second_file, "Hello duplicate file");
    writeFile(other_file, "Some other content");

    auto config = makeConfig(temp.path());
    config.masks.push_back("*.txt");

    const bayan::FileScanner scanner(config);
    const auto files = scanner.scan();

    const bayan::DuplicateFinder finder(config);
    const auto duplicate_groups = finder.find(files);

    const auto path_groups = toPathGroups(files, duplicate_groups);

    const std::set<std::string> expected_group = {
        canonicalString(first_file),
        canonicalString(second_file),
    };

    EXPECT_EQ(1U, path_groups.size());
    EXPECT_TRUE(containsGroup(path_groups, expected_group));
}

//------------------------------------------------------------------------------

TEST(DuplicateFinderTests, DoesNotTreatSameSizeFilesWithDifferentBlocksAsDuplicates)
{
    TemporaryDirectory temp;

    const auto first_file  = temp.path() / "first.txt";
    const auto second_file = temp.path() / "second.txt";

    writeFile(first_file, "HelloAAAAA");
    writeFile(second_file, "HelloBBBBB");

    auto config = makeConfig(temp.path());
    config.block_size = 5;

    const std::vector<bayan::FileInfo> files = {
        {canonicalString(first_file), fs::file_size(first_file)},
        {canonicalString(second_file), fs::file_size(second_file)},
    };

    const bayan::DuplicateFinder finder(config);
    const auto duplicate_groups = finder.find(files);

    EXPECT_TRUE(duplicate_groups.empty());
}

//------------------------------------------------------------------------------

TEST(DuplicateFinderTests, FindsGroupOfThreeDuplicateFiles)
{
    TemporaryDirectory temp;

    const auto first_file  = temp.path() / "first.txt";
    const auto second_file = temp.path() / "second.txt";
    const auto third_file  = temp.path() / "third.txt";
    const auto other_file  = temp.path() / "other.txt";

    writeFile(first_file, "abcde12345");
    writeFile(second_file, "abcde12345");
    writeFile(third_file, "abcde12345");
    writeFile(other_file, "abcde54321");

    auto config = makeConfig(temp.path());
    config.block_size = 5;

    const std::vector<bayan::FileInfo> files = {
        {canonicalString(first_file), fs::file_size(first_file)},
        {canonicalString(second_file), fs::file_size(second_file)},
        {canonicalString(third_file), fs::file_size(third_file)},
        {canonicalString(other_file), fs::file_size(other_file)},
    };

    const bayan::DuplicateFinder finder(config);
    const auto duplicate_groups = finder.find(files);

    const auto path_groups = toPathGroups(files, duplicate_groups);

    const std::set<std::string> expected_group = {
        canonicalString(first_file),
        canonicalString(second_file),
        canonicalString(third_file),
    };

    EXPECT_EQ(1U, path_groups.size());
    EXPECT_TRUE(containsGroup(path_groups, expected_group));
}

//------------------------------------------------------------------------------

TEST(FileScannerTests, AppliesMaskAndExcludeDirectory)
{
    TemporaryDirectory temp;

    const auto included_file = temp.path() / "included" / "file.txt";
    const auto excluded_file = temp.path() / "excluded" / "file.txt";
    const auto wrong_mask    = temp.path() / "included" / "file.cpp";

    writeFile(included_file, "content");
    writeFile(excluded_file, "content");
    writeFile(wrong_mask, "content");

    auto config = makeConfig(temp.path());
    config.exclude_dirs.push_back(temp.path() / "excluded");
    config.masks.push_back("*.txt");

    const bayan::FileScanner scanner(config);
    const auto files = scanner.scan();

    std::set<std::string> scanned_paths;

    for (const auto& file : files)
    {
        scanned_paths.insert(file.path.string());
    }

    EXPECT_TRUE(scanned_paths.count(canonicalString(included_file)) != 0);
    EXPECT_TRUE(scanned_paths.count(canonicalString(excluded_file)) == 0);
    EXPECT_TRUE(scanned_paths.count(canonicalString(wrong_mask)) == 0);
}

//------------------------------------------------------------------------------