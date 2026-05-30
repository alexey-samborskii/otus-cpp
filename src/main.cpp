#include "Database.hpp"
#include "Server.hpp"

#include <boost/asio.hpp>

#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <string>
#include <thread>
#include <vector>

//------------------------------------------------------------------------------

namespace
{

bool parsePort(const char* value, unsigned short& port)
{
    try
    {
        std::size_t pos    = 0;
        const auto  parsed = std::stoul(value, &pos);

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

std::size_t getThreadCount()
{
    const auto hardware_threads = std::thread::hardware_concurrency();

    if (hardware_threads == 0)
    {
        return 2;
    }

    return hardware_threads;
}

} // namespace

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>\n";
        return EXIT_FAILURE;
    }

    unsigned short port = 0;

    if (!parsePort(argv[1], port))
    {
        std::cerr << "Invalid port: " << argv[1] << '\n';
        return EXIT_FAILURE;
    }

    try
    {
        auto                    database = std::make_shared<Database>();
        boost::asio::io_context io_context;
        Server                  server(io_context, port, database);

        const auto               kThreadCount = getThreadCount();
        std::vector<std::thread> threads;

        threads.reserve(kThreadCount);

        for (std::size_t i = 0; i < kThreadCount; ++i)
        {
            threads.emplace_back([&io_context]() {
                io_context.run();
            });
        }

        for (auto& thread : threads)
        {
            thread.join();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}