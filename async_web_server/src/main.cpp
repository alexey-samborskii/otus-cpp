#include "server/HttpWebServer.hpp"
#include "server/HttpsWebServer.hpp"
#include "server/SslContext.hpp"
#include "server/WebServer.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <csignal>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace
{

enum class ServerProtocol
{
    kHttp  = 0,
    kHttps = 1
};

//------------------------------------------------------------------------------

struct ProgramOptions
{
    ServerProtocol        protocol   = ServerProtocol::kHttps;
    std::string           host       = "0.0.0.0";
    uint16_t              port       = 8443;
    bool                  port_set   = false;
    std::size_t           threads    = 4;
    std::filesystem::path public_dir = "public";
    std::filesystem::path cert_file  = "certs/server.crt";
    std::filesystem::path key_file   = "certs/server.key";
};

//------------------------------------------------------------------------------

ServerProtocol parseProtocol(const std::string &value)
{
    if (value == "http")
    {
        return ServerProtocol::kHttp;
    }

    if (value == "https")
    {
        return ServerProtocol::kHttps;
    }

    throw std::runtime_error(
        "Unsupported protocol: " + value + ". Expected: http or https");
}

//------------------------------------------------------------------------------

std::string toString(ServerProtocol protocol)
{
    switch (protocol)
    {
    case ServerProtocol::kHttp:
        return "http";

    case ServerProtocol::kHttps:
        return "https";
    }

    return "unknown";
}

//------------------------------------------------------------------------------

uint16_t parsePort(const std::string &value)
{
    const unsigned long port = std::stoul(value);

    if (port > 65535)
    {
        throw std::runtime_error("Port is out of range: " + value);
    }

    return static_cast<uint16_t>(port);
}

//------------------------------------------------------------------------------

std::size_t parseThreads(const std::string &value)
{
    std::size_t threads = static_cast<std::size_t>(std::stoul(value));

    if (threads == 0)
    {
        threads = 1;
    }

    return threads;
}

//------------------------------------------------------------------------------

ProgramOptions parseArguments(int argc, char **argv)
{
    ProgramOptions options;

    for (int i = 1; i < argc; ++i)
    {
        const std::string argument = argv[i];

        if (argument == "--protocol" && i + 1 < argc)
        {
            options.protocol = parseProtocol(argv[++i]);
        }
        else if (argument == "--http")
        {
            options.protocol = ServerProtocol::kHttp;
        }
        else if (argument == "--https")
        {
            options.protocol = ServerProtocol::kHttps;
        }
        else if (argument == "--host" && i + 1 < argc)
        {
            options.host = argv[++i];
        }
        else if (argument == "--port" && i + 1 < argc)
        {
            options.port     = parsePort(argv[++i]);
            options.port_set = true;
        }
        else if (argument == "--threads" && i + 1 < argc)
        {
            options.threads = parseThreads(argv[++i]);
        }
        else if (argument == "--public" && i + 1 < argc)
        {
            options.public_dir = argv[++i];
        }
        else if (argument == "--cert" && i + 1 < argc)
        {
            options.cert_file = argv[++i];
        }
        else if (argument == "--key" && i + 1 < argc)
        {
            options.key_file = argv[++i];
        }
        else
        {
            throw std::runtime_error(
                "Unknown or incomplete argument: " + argument);
        }
    }

    if (!options.port_set)
    {
        switch (options.protocol)
        {
        case ServerProtocol::kHttp:
            options.port = 8080;
            break;

        case ServerProtocol::kHttps:
            options.port = 8443;
            break;
        }
    }

    return options;
}

//------------------------------------------------------------------------------

void runServer(
    boost::asio::io_context &io_context,
    server::WebServer       &web_server,
    std::size_t              threads)
{
    boost::asio::signal_set signals(
        io_context,
        SIGINT,
        SIGTERM);

    signals.async_wait(
        [&web_server](const boost::system::error_code &, int signal) {
            std::cout << "\n[server] received signal "
                      << signal
                      << '\n';

            web_server.stop();
        });

    boost::asio::co_spawn(
        io_context,
        web_server.acceptLoop(),
        boost::asio::detached);

    std::vector<std::thread> workers;

    workers.reserve(threads);

    for (std::size_t i = 0; i < threads; ++i)
    {
        workers.emplace_back(
            [&io_context]() {
                io_context.run();
            });
    }

    for (std::thread &worker : workers)
    {
        worker.join();
    }
}

} // namespace

//------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    try
    {
        const ProgramOptions options = parseArguments(
            argc,
            argv);

        const auto address = boost::asio::ip::make_address(options.host);

        std::cout << "[server] protocol: "
                  << toString(options.protocol)
                  << '\n';

        std::cout << "[server] address: "
                  << options.host
                  << ':'
                  << options.port
                  << '\n';

        std::cout << "[server] public dir: "
                  << options.public_dir
                  << '\n';

        boost::asio::io_context io_context;

        switch (options.protocol)
        {
        case ServerProtocol::kHttp:
        {
            server::HttpWebServer web_server(
                io_context,
                {address, options.port},
                options.public_dir);

            runServer(
                io_context,
                web_server,
                options.threads);

            break;
        }

        case ServerProtocol::kHttps:
        {
            boost::asio::ssl::context ssl_context(
                boost::asio::ssl::context::tls_server);

            server::configureServerSslContext(
                ssl_context,
                options.cert_file,
                options.key_file);

            server::HttpsWebServer web_server(
                io_context,
                {address, options.port},
                options.public_dir,
                ssl_context);

            runServer(
                io_context,
                web_server,
                options.threads);

            break;
        }
        }

        std::cout << "[server] stopped\n";
    }
    catch (const std::exception &error)
    {
        std::cerr << "[error] "
                  << error.what()
                  << '\n';

        return 1;
    }

    return 0;
}

//------------------------------------------------------------------------------