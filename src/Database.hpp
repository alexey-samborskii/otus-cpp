#pragma once

#include <map>
#include <mutex>
#include <string>

//------------------------------------------------------------------------------

class Database
{
public:
    enum class Table
    {
        kA,
        kB
    };

    bool insert(Table table, int id, const std::string& name);

    void truncate(Table table);

    std::string intersection() const;

    std::string symmetricDifference() const;

private:
    using Records = std::map<int, std::string>;

    Records&       getTable(Table table);
    const Records& getTable(Table table) const;

private:
    mutable std::mutex mutex_;
    Records            table_a_;
    Records            table_b_;
};