#include "Server.hpp"

#include <boost/asio.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <string>

//------------------------------------------------------------------------------

namespace
{

bool parsePort(const char* value, unsigned short& port)
{
    try
    {
        std::size_t pos = 0;
        const auto parsed = std::stoul(value, &pos);

        if (value[pos] != '\0' ||
            parsed == 0 ||
            parsed > std::numeric_limits<unsigned short>::max())
        {
            return false;
        }

        port = static_cast<unsigned short>(parsed);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

//------------------------------------------------------------------------------

bool parseBulkSize(const char* value, std::size_t& bulk_size)
{
    try
    {
        std::size_t pos = 0;
        const auto parsed = std::stoul(value, &pos);

        if (value[pos] != '\0' || parsed == 0)
        {
            return false;
        }

        bulk_size = static_cast<std::size_t>(parsed);
        return true;
    }
    catch (...)
    {
        return false;
    }
}

} // namespace

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: " << argv[0] << " <port> <bulk_size>\n";
        return EXIT_FAILURE;
    }

    unsigned short port = 0;
    std::size_t bulk_size = 0;

    if (!parsePort(argv[1], port))
    {
        std::cerr << "Invalid port: " << argv[1] << '\n';
        return EXIT_FAILURE;
    }

    if (!parseBulkSize(argv[2], bulk_size))
    {
        std::cerr << "Invalid bulk size: " << argv[2] << '\n';
        return EXIT_FAILURE;
    }

    try
    {
        boost::asio::io_context io_context;

        Server server(io_context, port, bulk_size);

        io_context.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}