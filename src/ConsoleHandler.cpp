#include "ConsoleHandler.hpp"

#include "Formatter.hpp"

#include <iostream>

namespace bulk
{

void ConsoleHandler::handle(const CommandBlock& bulk)
{
    Formatter::write(std::cout, bulk);
}

} // namespace bulk