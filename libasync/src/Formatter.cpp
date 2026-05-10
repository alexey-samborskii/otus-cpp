#include "Formatter.hpp"

#include <ostream>
#include <sstream>

namespace bulk
{
    
//------------------------------------------------------------------------------

std::string Formatter::format(const CommandBlock& bulk)
{
    std::ostringstream result;

    result << "bulk: ";

    for (std::size_t i = 0; i < bulk.commands.size(); ++i)
    {
        if (i != 0)
        {
            result << ", ";
        }

        result << bulk.commands[i];
    }

    return result.str();
}

//------------------------------------------------------------------------------

void Formatter::write(std::ostream& output, const CommandBlock& bulk)
{
    output << format(bulk) << '\n';
}

//------------------------------------------------------------------------------

} // namespace bulk