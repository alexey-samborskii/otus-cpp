#include "CommandProcessor.hpp"

#include <stdexcept>
#include <sstream>
#include <utility>
#include <vector>

//------------------------------------------------------------------------------

namespace
{

std::vector<std::string> splitBySingleSpace(const std::string& command)
{
    std::vector<std::string> parts;

    std::size_t start = 0;

    while (start <= command.size())
    {
        const auto pos = command.find(' ', start);

        if (pos == std::string::npos)
        {
            parts.push_back(command.substr(start));
            break;
        }

        parts.push_back(command.substr(start, pos - start));
        start = pos + 1;
    }

    return parts;
}

//------------------------------------------------------------------------------

bool hasEmptyParts(const std::vector<std::string>& parts)
{
    for (const auto& part : parts)
    {
        if (part.empty())
        {
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------

std::string makeError(const std::string& message)
{
    return "ERR " + message + '\n';
}

} // namespace

//------------------------------------------------------------------------------

CommandProcessor::CommandProcessor(std::shared_ptr<Database> database)
    : database_(std::move(database))
{
    if (!database_)
    {
        throw std::invalid_argument("Database is null");
    }
}

//------------------------------------------------------------------------------

std::string CommandProcessor::process(const std::string& command)
{
    if (command.empty())
    {
        return makeError("invalid command");
    }

    const auto parts = splitBySingleSpace(command);

    if (hasEmptyParts(parts))
    {
        return makeError("invalid command");
    }

    if (parts[0] == "INSERT")
    {
        return processInsert(command);
    }

    if (parts[0] == "TRUNCATE")
    {
        return processTruncate(command);
    }

    if (parts[0] == "INTERSECTION")
    {
        if (parts.size() != 1)
        {
            return makeError("invalid command");
        }

        return database_->intersection();
    }

    if (parts[0] == "SYMMETRIC_DIFFERENCE")
    {
        if (parts.size() != 1)
        {
            return makeError("invalid command");
        }

        return database_->symmetricDifference();
    }

    return makeError("unknown command");
}

//------------------------------------------------------------------------------

std::string CommandProcessor::processInsert(const std::string& command)
{
    const auto parts = splitBySingleSpace(command);

    if (parts.size() != 4 || hasEmptyParts(parts))
    {
        return makeError("invalid command");
    }

    Database::Table table;

    if (!parseTable(parts[1], table))
    {
        return makeError("unknown table");
    }

    int id = 0;

    if (!parseInt(parts[2], id))
    {
        return makeError("invalid id");
    }

    const auto& name = parts[3];

    if (!database_->insert(table, id, name))
    {
        return makeError("duplicate " + std::to_string(id));
    }

    return "OK\n";
}

//------------------------------------------------------------------------------

std::string CommandProcessor::processTruncate(const std::string& command)
{
    const auto parts = splitBySingleSpace(command);

    if (parts.size() != 2 || hasEmptyParts(parts))
    {
        return makeError("invalid command");
    }

    Database::Table table;

    if (!parseTable(parts[1], table))
    {
        return makeError("unknown table");
    }

    database_->truncate(table);

    return "OK\n";
}

//------------------------------------------------------------------------------

bool CommandProcessor::parseTable(const std::string& value, Database::Table& table)
{
    if (value == "A")
    {
        table = Database::Table::kA;
        return true;
    }

    if (value == "B")
    {
        table = Database::Table::kB;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------

bool CommandProcessor::parseInt(const std::string& value, int& result)
{
    try
    {
        std::size_t pos    = 0;
        const auto  parsed = std::stoi(value, &pos);

        if (pos != value.size())
        {
            return false;
        }

        result = parsed;
        return true;
    }
    catch (...)
    {
        return false;
    }
}