#include "AppConfig.hpp"
#include "ApplicationRequestHandler.hpp"
#include "TaskApiHandler.hpp"

#include "server/HttpRequestHandler.hpp"
#include "server/HttpWebServer.hpp"
#include "server/HttpsWebServer.hpp"
#include "server/StaticFileHandler.hpp"
#include "server/WebServer.hpp"

#include "tasks/TaskRepository.hpp"
#include "tasks/TaskScheduler.hpp"
#include "tasks/TaskService.hpp"

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

namespace net = boost::asio;
namespace ssl = boost::asio::ssl;

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
    ServerProtocol        protocol = ServerProtocol::kHttps;
    std::string           host     = "0.0.0.0";
    std::uint16_t         port     = 8443;
    bool                  port_set = false;
    std::size_t           threads  = 4;
    std::filesystem::path public_dir;
    bool                  public_dir_set   = false;
    std::filesystem::path database_file    = "data/tasks.db";
    std::filesystem::path certificate_file = "certs/server.crt";
    std::filesystem::path private_key_file = "certs/server.key";
};

using HttpRequestHandlerPtr = std::shared_ptr<server::HttpRequestHandler>;
using WebServerPtr          = std::shared_ptr<server::WebServer>;

//------------------------------------------------------------------------------

void printUsage(const char *program_name)
{
    std::cout
        << "Usage: " << program_name << " [options]\n\n"
        << "Options:\n"
        << "  --protocol <http|https>  Server protocol, default: https\n"
        << "  --host <address>         Bind address, default: 0.0.0.0\n"
        << "  --port <number>          Bind port, default: 8080/8443\n"
        << "  --threads <number>       io_context worker threads, default: 4\n"
        << "  --public-dir <path>      Web interface directory\n"
        << "                           Default: automatic detection\n"
        << "  --database <path>        SQLite database file\n"
        << "  --cert <path>            TLS certificate file\n"
        << "  --key <path>             TLS private key file\n"
        << "  --help                   Show this help\n";
}

//------------------------------------------------------------------------------

std::string requireValue(
    int         argc,
    char       *argv[],
    int        &index,
    const char *option_name)
{
    if (index + 1 >= argc)
    {
        throw std::invalid_argument(
            std::string("Missing value for ") + option_name);
    }

    ++index;

    return argv[index];
}

//------------------------------------------------------------------------------

std::uint16_t parsePort(const std::string &value)
{
    std::size_t parsed_characters = 0;

    const unsigned long parsed = std::stoul(value, &parsed_characters);

    if (parsed_characters != value.size())
    {
        throw std::invalid_argument("Port must contain only digits");
    }

    if (parsed == 0 || parsed > 65535)
    {
        throw std::invalid_argument("Port must be in range 1..65535");
    }

    return static_cast<std::uint16_t>(parsed);
}

//------------------------------------------------------------------------------

std::size_t parseThreadCount(const std::string &value)
{
    std::size_t parsed_characters = 0;

    const unsigned long parsed = std::stoul(
        value,
        &parsed_characters);

    if (parsed_characters != value.size())
    {
        throw std::invalid_argument("Thread count must contain only digits");
    }

    if (parsed == 0)
    {
        throw std::invalid_argument("Thread count must be greater than zero");
    }

    return static_cast<std::size_t>(parsed);
}

//------------------------------------------------------------------------------

ProgramOptions parseProgramOptions(
    int   argc,
    char *argv[])
{
    ProgramOptions options;

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--help")
        {
            printUsage(argv[0]);
            std::exit(EXIT_SUCCESS);
        }

        if (argument == "--protocol")
        {
            const std::string value =
                requireValue(argc, argv, index, "--protocol");

            if (value == "http")
            {
                options.protocol = ServerProtocol::kHttp;
            }
            else if (value == "https")
            {
                options.protocol = ServerProtocol::kHttps;
            }
            else
            {
                throw std::invalid_argument(
                    "Protocol must be either http or https");
            }

            continue;
        }

        if (argument == "--host")
        {
            options.host = requireValue(argc, argv, index, "--host");
            continue;
        }

        if (argument == "--port")
        {
            options.port =
                parsePort(requireValue(argc, argv, index, "--port"));
            options.port_set = true;
            continue;
        }

        if (argument == "--threads")
        {
            options.threads =
                parseThreadCount(requireValue(argc, argv, index, "--threads"));
            continue;
        }

        if (argument == "--public-dir")
        {
            options.public_dir =
                requireValue(argc, argv, index, "--public-dir");
            continue;
        }

        if (argument == "--database")
        {
            options.database_file =
                requireValue(argc, argv, index, "--database");
            continue;
        }

        if (argument == "--cert")
        {
            options.certificate_file =
                requireValue(argc, argv, index, "--cert");
            continue;
        }

        if (argument == "--key")
        {
            options.private_key_file = requireValue(argc, argv, index, "--key");
            continue;
        }

        throw std::invalid_argument(
            "Unknown command-line option: " + argument);
    }

    if (!options.port_set)
    {
        options.port = options.protocol == ServerProtocol::kHttps ? 8443 : 8080;
    }

    return options;
}

//------------------------------------------------------------------------------

std::filesystem::path executablePath()
{
    std::error_code error;

    const std::filesystem::path path =
        std::filesystem::read_symlink("/proc/self/exe", error);

    if (error)
    {
        return {};
    }

    return path;
}

//------------------------------------------------------------------------------

std::filesystem::path localPublicDirectory()
{
    const std::filesystem::path executable_path = executablePath();

    if (executable_path.empty())
    {
        return {};
    }

    return executable_path.parent_path() / "public";
}

//------------------------------------------------------------------------------

std::filesystem::path installedPublicDirectory()
{
    const std::filesystem::path executable_path = executablePath();

    if (executable_path.empty())
    {
        return {};
    }

    std::filesystem::path install_prefix =
        executable_path.parent_path();

    const std::filesystem::path binary_directory(
        application::config::kInstallBinaryDirectory);

    for (const std::filesystem::path &component : binary_directory)
    {
        if (component.empty() || component == ".")
        {
            continue;
        }

        install_prefix = install_prefix.parent_path();
    }

    return install_prefix /
           application::config::kInstallPublicDirectory;
}

//------------------------------------------------------------------------------

bool isUsablePublicDirectory(
    const std::filesystem::path &directory)
{
    if (directory.empty())
    {
        return false;
    }

    std::error_code error;

    if (!std::filesystem::is_directory(directory, error) || error)
    {
        return false;
    }

    std::ifstream index_file(
        directory / "index.html",
        std::ios::binary);

    return index_file.good();
}

//------------------------------------------------------------------------------

std::filesystem::path normalizePath(
    const std::filesystem::path &path)
{
    std::error_code error;

    const std::filesystem::path normalized_path =
        std::filesystem::weakly_canonical(path, error);

    if (error)
    {
        return path.lexically_normal();
    }

    return normalized_path;
}

//------------------------------------------------------------------------------

std::filesystem::path resolvePublicDirectory(
    const ProgramOptions &options)
{
    if (options.public_dir_set)
    {
        if (isUsablePublicDirectory(options.public_dir))
        {
            return normalizePath(options.public_dir);
        }

        std::cerr
            << "Public directory specified by --public-dir is unavailable: "
            << options.public_dir
            << "\nTrying fallback directories\n";
    }

    const std::filesystem::path local_directory =
        localPublicDirectory();

    if (isUsablePublicDirectory(local_directory))
    {
        return normalizePath(local_directory);
    }

    const std::filesystem::path installed_directory =
        installedPublicDirectory();

    if (isUsablePublicDirectory(installed_directory))
    {
        return normalizePath(installed_directory);
    }

    std::string message =
        "Unable to locate a readable public directory";

    if (options.public_dir_set)
    {
        message +=
            "\nRequested directory: " +
            options.public_dir.string();
    }

    if (!local_directory.empty())
    {
        message +=
            "\nLocal directory: " +
            local_directory.string();
    }

    if (!installed_directory.empty())
    {
        message +=
            "\nInstalled directory: " +
            installed_directory.string();
    }

    throw std::runtime_error(message);
}

//------------------------------------------------------------------------------

WebServerPtr createWebServer(
    const ProgramOptions        &options,
    net::io_context             &io_context,
    const HttpRequestHandlerPtr &request_handler)
{
    const net::ip::address address = net::ip::make_address(options.host);

    const net::ip::tcp::endpoint endpoint(address, options.port);

    if (options.protocol == ServerProtocol::kHttps)
    {
        auto ssl_context = std::make_shared<ssl::context>(
            ssl::context::tls_server);

        ssl_context->set_options(
            ssl::context::default_workarounds |
            ssl::context::no_sslv2 |
            ssl::context::no_sslv3 |
            ssl::context::no_tlsv1 |
            ssl::context::no_tlsv1_1 |
            ssl::context::single_dh_use);

        ssl_context->use_certificate_chain_file(
            options.certificate_file.string());

        ssl_context->use_private_key_file(
            options.private_key_file.string(),
            ssl::context::pem);

        return std::make_shared<server::HttpsWebServer>(
            io_context,
            endpoint,
            std::move(ssl_context),
            request_handler);
    }

    return std::make_shared<server::HttpWebServer>(
        io_context,
        endpoint,
        request_handler);
}

//------------------------------------------------------------------------------

void setupSignalHandling(
    net::signal_set    &signals,
    net::io_context    &io_context,
    const WebServerPtr &web_server)
{
    signals.async_wait(
        [&io_context, web_server](
            const boost::system::error_code &error,
            int                              signal_number) {
            if (error)
            {
                return;
            }

            std::cout
                << "Received signal "
                << signal_number
                << ", stopping server\n";

            web_server->stop();
            io_context.stop();
        });
}

//------------------------------------------------------------------------------

void restoreScheduledTasks(
    net::io_context                             &io_context,
    const std::shared_ptr<tasks::TaskScheduler> &scheduler)
{
    net::co_spawn(
        io_context,
        scheduler->restore(),
        [](std::exception_ptr error) {
            if (!error)
            {
                return;
            }

            try
            {
                std::rethrow_exception(error);
            }
            catch (const std::exception &exception)
            {
                std::cerr
                    << "[scheduler] restore error: "
                    << exception.what()
                    << '\n';
            }
        });
}

//------------------------------------------------------------------------------

void runWebServer(
    const ProgramOptions &options,
    net::io_context      &io_context,
    const WebServerPtr   &web_server)
{
    net::co_spawn(
        io_context,
        [web_server]() -> net::awaitable<void> {
            co_await web_server->acceptLoop();
        },
        [&io_context](std::exception_ptr error) {
            if (error)
            {
                try
                {
                    std::rethrow_exception(error);
                }
                catch (const std::exception &exception)
                {
                    std::cerr
                        << "[server] accept loop error: "
                        << exception.what()
                        << '\n';
                }
            }

            io_context.stop();
        });

    const char *protocol_name =
        options.protocol == ServerProtocol::kHttps ? "https" : "http";

    std::cout
        << "Server started: "
        << protocol_name
        << "://"
        << options.host
        << ':'
        << options.port
        << "\nPublic directory: "
        << options.public_dir
        << "\nSQLite database: "
        << options.database_file
        << "\nWorker threads: "
        << options.threads
        << '\n';

    std::vector<std::thread> workers;

    workers.reserve(options.threads - 1);

    for (std::size_t index = 1; index < options.threads; ++index)
    {
        workers.emplace_back(
            [&io_context]() {
                io_context.run();
            });
    }

    io_context.run();

    for (std::thread &worker : workers)
    {
        worker.join();
    }
}

} // namespace

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        ProgramOptions options = parseProgramOptions(argc, argv);

        options.public_dir = resolvePublicDirectory(options);

        net::io_context io_context;

        tasks::TaskRepository repository(options.database_file);

        auto scheduler = std::make_shared<tasks::TaskScheduler>(
            io_context.get_executor(),
            repository);

        auto task_service = std::make_shared<tasks::TaskService>(
            repository,
            scheduler);

        auto task_api_handler =
            std::make_shared<application::TaskApiHandler>(
                task_service);

        auto request_handler =
            std::make_shared<application::ApplicationRequestHandler>(
                server::StaticFileHandler(options.public_dir),
                task_api_handler);

        const auto web_server = createWebServer(
            options,
            io_context,
            request_handler);

        net::signal_set signals(io_context, SIGINT, SIGTERM);

        setupSignalHandling(signals, io_context, web_server);

        restoreScheduledTasks(io_context, scheduler);

        runWebServer(options, io_context, web_server);

        return EXIT_SUCCESS;
    }
    catch (const std::exception &error)
    {
        std::cerr
            << "Fatal error: "
            << error.what()
            << '\n';
    }

    return EXIT_FAILURE;
}

//------------------------------------------------------------------------------