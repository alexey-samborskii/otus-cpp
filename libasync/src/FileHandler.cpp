#include "FileHandler.hpp"

#include "Formatter.hpp"

#include <fstream>
#include <sstream>
#include <stdexcept>

namespace bulk
{

namespace
{
    
std::string makeFileName(std::time_t timestamp)
{
    std::ostringstream result;
    result << "bulk" << timestamp << ".log";
    return result.str();
}

//------------------------------------------------------------------------------

std::string makeFilePath(const std::string& dir, const std::string& file_name)
{
    if (dir.empty() || dir == ".")
    {
        return file_name;
    }

    const char last = dir[dir.size() - 1];

    if (last == '/' || last == '\\')
    {
        return dir + file_name;
    }

    return dir + "/" + file_name;
}

} // namespace

//------------------------------------------------------------------------------

FileHandler::FileHandler(std::string output_dir)
    : output_dir_(std::move(output_dir))
{
}

//------------------------------------------------------------------------------

void FileHandler::handle(const CommandBlock& bulk)
{
    const std::string file_path =
        makeFilePath(output_dir_, makeFileName(bulk.timestamp));

    std::ofstream file(file_path.c_str());

    if (!file)
    {
        throw std::runtime_error("failed to open log file: " + file_path);
    }

    Formatter::write(file, bulk);
}

//------------------------------------------------------------------------------

} // namespace bulk