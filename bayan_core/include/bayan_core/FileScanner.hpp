#pragma once

#include "Config.hpp"
#include "FileInfo.hpp"

#include <vector>

namespace bayan
{

//------------------------------------------------------------------------------

class FileScanner
{
public:
    explicit FileScanner(Config config);

    std::vector<FileInfo> scan() const;

private:
    void collectFromDirectory(const boost::filesystem::path& directory,
                              std::vector<FileInfo>&         files) const;

    bool isExcluded(const boost::filesystem::path& path) const;
    bool matchesMasks(const boost::filesystem::path& path) const;

private:
    Config config_;
};

//------------------------------------------------------------------------------

} // namespace bayan