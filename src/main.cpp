#include "CommandBlockProcessor.hpp"
#include "CompositeHandler.hpp"
#include "ConsoleHandler.hpp"
#include "FileHandler.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <block_size>\n";
        return EXIT_FAILURE;
    }

    const std::string block_size_arg = argv[1];

    if (block_size_arg.empty() ||
        block_size_arg == "0" ||
        block_size_arg.find_first_not_of("0123456789") != std::string::npos)
    {
        std::cerr << "Invalid block size: " << argv[1] << '\n';
        return EXIT_FAILURE;
    }

    const auto block_size = static_cast<std::size_t>(std::stoul(block_size_arg));

    try
    {
        bulk::CompositeHandler handler;
        handler.add(std::make_shared<bulk::ConsoleHandler>());
        handler.add(std::make_shared<bulk::FileHandler>());

        bulk::CommandBlockProcessor processor(block_size, handler);

        std::string line;
        while (std::getline(std::cin, line))
        {
            processor.processLine(std::move(line));
        }

        processor.finish();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

//------------------------------------------------------------------------------