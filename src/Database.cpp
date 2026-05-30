#include "Database.hpp"

#include <sstream>

//------------------------------------------------------------------------------

bool Database::insert(Table table, int id, const std::string& name)
{
    std::lock_guard<std::mutex> lock(mutex_);

    auto& records = getTable(table);

    const auto [it, inserted] = records.emplace(id, name);

    return inserted;
}

//------------------------------------------------------------------------------

void Database::truncate(Table table)
{
    std::lock_guard<std::mutex> lock(mutex_);

    getTable(table).clear();
}

//------------------------------------------------------------------------------

std::string Database::intersection() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream result;

    auto it_a = table_a_.begin();
    auto it_b = table_b_.begin();

    while (it_a != table_a_.end() && it_b != table_b_.end())
    {
        if (it_a->first < it_b->first)
        {
            ++it_a;
            continue;
        }

        if (it_b->first < it_a->first)
        {
            ++it_b;
            continue;
        }

        result << it_a->first << ','
               << it_a->second << ','
               << it_b->second << '\n';

        ++it_a;
        ++it_b;
    }

    result << "OK\n";

    return result.str();
}

//------------------------------------------------------------------------------

std::string Database::symmetricDifference() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    std::ostringstream result;

    auto it_a = table_a_.begin();
    auto it_b = table_b_.begin();

    while (it_a != table_a_.end() || it_b != table_b_.end())
    {
        if (it_b == table_b_.end() ||
            (it_a != table_a_.end() && it_a->first < it_b->first))
        {
            result << it_a->first << ','
                   << it_a->second << ",\n";

            ++it_a;
            continue;
        }

        if (it_a == table_a_.end() || it_b->first < it_a->first)
        {
            result << it_b->first << ",,"
                   << it_b->second << '\n';

            ++it_b;
            continue;
        }

        ++it_a;
        ++it_b;
    }

    result << "OK\n";

    return result.str();
}

//------------------------------------------------------------------------------

Database::Records& Database::getTable(Table table)
{
    switch (table)
    {
    case Table::kA:
        return table_a_;

    case Table::kB:
        return table_b_;
    }

    return table_a_;
}

//------------------------------------------------------------------------------

const Database::Records& Database::getTable(Table table) const
{
    switch (table)
    {
    case Table::kA:
        return table_a_;

    case Table::kB:
        return table_b_;
    }

    return table_a_;
}