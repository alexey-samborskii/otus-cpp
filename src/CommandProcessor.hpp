#pragma once

#include "Database.hpp"

#include <memory>
#include <string>

//------------------------------------------------------------------------------

class CommandProcessor
{
public:
    explicit CommandProcessor(std::shared_ptr<Database> database);

    std::string process(const std::string& command);

private:
    std::string processInsert(const std::string& command);
    std::string processTruncate(const std::string& command);

    static bool parseTable(const std::string& value, Database::Table& table);

    static bool parseInt(const std::string& value, int& result);

private:
    std::shared_ptr<Database> database_;
};