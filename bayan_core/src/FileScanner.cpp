#include "bayan_core/FileScanner.hpp"

#include <algorithm>
#include <iostream>

namespace bayan
{

namespace
{

namespace fs = boost::filesystem;

//------------------------------------------------------------------------------

fs::path normalizePath(const fs::path& path)
{
    boost::system::error_code ec;

    const auto canonical_path = fs::canonical(path, ec);
    if (!ec)
    {
        return canonical_path;
    }

    const auto absolute_path = fs::absolute(path, ec);
    if (!ec)
    {
        return absolute_path;
    }

    return path;
}

//------------------------------------------------------------------------------

bool isSameOrChildPath(const fs::path& path, const fs::path& parent)
{
    const auto normalized_path   = normalizePath(path);
    const auto normalized_parent = normalizePath(parent);

    auto path_it   = normalized_path.begin();
    auto parent_it = normalized_parent.begin();

    for (; parent_it != normalized_parent.end(); ++parent_it, ++path_it)
    {
        if (path_it == normalized_path.end() || *path_it != *parent_it)
        {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------

bool wildcardMatch(const std::string& text, const std::string& pattern)
{
    std::size_t text_pos    = 0;
    std::size_t pattern_pos = 0;
    std::size_t star_pos    = std::string::npos;
    std::size_t match_pos   = 0;

    while (text_pos < text.size())
    {
        if (pattern_pos < pattern.size() &&
            (pattern[pattern_pos] == '?' ||
             pattern[pattern_pos] == text[text_pos]))
        {
            ++text_pos;
            ++pattern_pos;
            continue;
        }

        if (pattern_pos < pattern.size() && pattern[pattern_pos] == '*')
        {
            star_pos  = pattern_pos;
            match_pos = text_pos;

            ++pattern_pos;
            continue;
        }

        if (star_pos != std::string::npos)
        {
            pattern_pos = star_pos + 1;
            text_pos    = ++match_pos;
            continue;
        }

        return false;
    }

    while (pattern_pos < pattern.size() && pattern[pattern_pos] == '*')
    {
        ++pattern_pos;
    }

    return pattern_pos == pattern.size();
}

} // namespace

//------------------------------------------------------------------------------

FileScanner::FileScanner(Config config)
    : config_(std::move(config))
{
}

//------------------------------------------------------------------------------

std::vector<FileInfo> FileScanner::scan() const
{
    std::vector<FileInfo> files;

    for (const auto& directory : config_.scan_dirs)
    {
        collectFromDirectory(directory, files);
    }

    return files;
}

//------------------------------------------------------------------------------

void FileScanner::collectFromDirectory(const fs::path&        directory,
                                       std::vector<FileInfo>& files) const
{
    boost::system::error_code ec;

    if (!fs::exists(directory, ec) || !fs::is_directory(directory, ec))
    {
        std::cerr << "Directory does not exist: "
                  << directory.string()
                  << '\n';
        return;
    }

    if (isExcluded(directory))
    {
        return;
    }

    fs::recursive_directory_iterator it(directory, ec);
    fs::recursive_directory_iterator end;

    while (!ec && it != end)
    {
        const auto path = it->path();

        if (fs::is_directory(path, ec))
        {
            if (isExcluded(path))
            {
                it.disable_recursion_pending();
            }
            else if (config_.depth >= 0 && it.depth() >= config_.depth)
            {
                it.disable_recursion_pending();
            }

            it.increment(ec);
            continue;
        }

        if (!fs::is_regular_file(path, ec))
        {
            it.increment(ec);
            continue;
        }

        if (isExcluded(path) || !matchesMasks(path))
        {
            it.increment(ec);
            continue;
        }

        const auto size = fs::file_size(path, ec);
        if (!ec && size > config_.min_size)
        {
            files.push_back({normalizePath(path), size});
        }

        it.increment(ec);
    }

    if (ec)
    {
        std::cerr << "Directory scan error: "
                  << directory.string()
                  << ": "
                  << ec.message()
                  << '\n';
    }
}

//------------------------------------------------------------------------------

bool FileScanner::isExcluded(const fs::path& path) const
{
    return std::any_of(
        config_.exclude_dirs.begin(),
        config_.exclude_dirs.end(),
        [&path](const fs::path& exclude_dir) {
            return isSameOrChildPath(path, exclude_dir);
        });
}

//------------------------------------------------------------------------------

bool FileScanner::matchesMasks(const fs::path& path) const
{
    if (config_.masks.empty())
    {
        return true;
    }

    const auto filename = path.filename().string();

    return std::any_of(
        config_.masks.begin(),
        config_.masks.end(),
        [&filename](const std::string& mask) {
            return wildcardMatch(filename, mask);
        });
}

//------------------------------------------------------------------------------

} // namespace bayan