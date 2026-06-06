#pragma once

#include "project_config.hpp"

#include <fstream>
#include <stdexcept>
#include <string>

//------------------------------------------------------------------------------

inline bool fileExists(const std::string &file_name)
{
    std::ifstream file(file_name.c_str());
    return file.good();
}

//------------------------------------------------------------------------------

inline bool isAbsolutePath(const std::string &path)
{
    return !path.empty() && path[0] == '/';
}

//------------------------------------------------------------------------------

inline std::string joinPath(
    const std::string &dir,
    const std::string &path)
{
    if (dir.empty())
    {
        return path;
    }

    if (dir[dir.size() - 1] == '/')
    {
        return dir + path;
    }

    return dir + "/" + path;
}

//------------------------------------------------------------------------------

inline std::string resolveInputPath(const std::string &path)
{
    if (fileExists(path))
    {
        return path;
    }

    if (!isAbsolutePath(path))
    {
        const std::string installed_path =
            joinPath(PROJECT_INSTALL_DATA_DIR, path);

        if (fileExists(installed_path))
        {
            return installed_path;
        }
    }

    throw std::runtime_error("Cannot find file: " + path);
}

//------------------------------------------------------------------------------