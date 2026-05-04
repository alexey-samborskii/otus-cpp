#include "bayan_core/DuplicateFinder.hpp"
#include "bayan_core/FileScanner.hpp"
#include "bayan_core/ArgumentsParser.hpp"
#include "bayan_core/OutputPrinter.hpp"

#include <exception>
#include <iostream>

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    try
    {
        using namespace bayan;

        const ArgumentsParser args_parser;
        const auto            config = args_parser.parse(argc, argv);

        const FileScanner file_scanner(config);
        const auto        files = file_scanner.scan();

        const DuplicateFinder duplicate_finder(config);
        const auto            duplicate_groups = duplicate_finder.find(files);

        const OutputPrinter output_printer;
        output_printer.print(files, duplicate_groups);
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}

//------------------------------------------------------------------------------