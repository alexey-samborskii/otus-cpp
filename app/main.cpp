#include "AppConfig.hpp"
#include "TaskApiHandler.hpp"

#include "server/common.hpp"
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
        << "Usage: " << program_name << " [config]\n\n"
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
    ProgramOptions config;

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
                config.protocol = ServerProtocol::kHttp;
            }
            else if (value == "https")
            {
                config.protocol = ServerProtocol::kHttps;
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
            config.host = requireValue(argc, argv, index, "--host");
            continue;
        }

        if (argument == "--port")
        {
            config.port =
                parsePort(requireValue(argc, argv, index, "--port"));
            config.port_set = true;
            continue;
        }

        if (argument == "--threads")
        {
            config.threads =
                parseThreadCount(requireValue(argc, argv, index, "--threads"));
            continue;
        }

        if (argument == "--public-dir")
        {
            config.public_dir =
                requireValue(argc, argv, index, "--public-dir");
            continue;
        }

        if (argument == "--database")
        {
            config.database_file =
                requireValue(argc, argv, index, "--database");
            continue;
        }

        if (argument == "--cert")
        {
            config.certificate_file =
                requireValue(argc, argv, index, "--cert");
            continue;
        }

        if (argument == "--key")
        {
            config.private_key_file = requireValue(argc, argv, index, "--key");
            continue;
        }

        throw std::invalid_argument(
            "Unknown command-line option: " + argument);
    }

    if (!config.port_set)
    {
        config.port = config.protocol == ServerProtocol::kHttps ? 8443 : 8080;
    }

    return config;
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
    const ProgramOptions &config)
{
    if (config.public_dir_set)
    {
        if (isUsablePublicDirectory(config.public_dir))
        {
            return normalizePath(config.public_dir);
        }

        std::cerr
            << "Public directory specified by --public-dir is unavailable: "
            << config.public_dir
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

    if (config.public_dir_set)
    {
        message +=
            "\nRequested directory: " +
            config.public_dir.string();
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
    const ProgramOptions         &config,
    net::io_context              &io_context,
    server::CallbackHandleRequest request_handler)
{
    const net::ip::address address = net::ip::make_address(config.host);

    const net::ip::tcp::endpoint endpoint(address, config.port);

    if (config.protocol == ServerProtocol::kHttp)
    {
        return std::make_shared<server::HttpWebServer>(
            io_context,
            endpoint,
            std::move(request_handler));
    }

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
        config.certificate_file.string());

    ssl_context->use_private_key_file(
        config.private_key_file.string(),
        ssl::context::pem);

    return std::make_shared<server::HttpsWebServer>(
        io_context,
        endpoint,
        std::move(ssl_context),
        std::move(request_handler));
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
    const ProgramOptions &config,
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
        config.protocol == ServerProtocol::kHttps ? "https" : "http";

    std::cout
        << "Server started: "
        << protocol_name
        << "://"
        << config.host
        << ':'
        << config.port
        << "\nPublic directory: "
        << config.public_dir
        << "\nSQLite database: "
        << config.database_file
        << "\nWorker threads: "
        << config.threads
        << '\n';

    std::vector<std::thread> workers;

    workers.reserve(config.threads - 1);

    for (std::size_t index = 1; index < config.threads; ++index)
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

//------------------------------------------------------------------------------

server::HttpResponse makeInternalServerError(unsigned int version)
{
    server::HttpResponse response;

    response.status       = 500;
    response.version      = version;
    response.content_type = "text/plain; charset=utf-8";
    response.keep_alive   = false;
    response.body         = "Internal Server Error";

    return response;
}

//------------------------------------------------------------------------------

server::CallbackHandleRequest withExceptionHandling(
    server::CallbackHandleRequest request_handler)
{
    return [request_handler = std::move(request_handler)](
               server::HttpRequest &&request) mutable
               -> server::AwaitableResponse {
        const auto request_version = request.version;

        try
        {
            co_return co_await request_handler(std::move(request));
        }
        catch (const boost::system::system_error &error)
        {
            if (error.code() == net::error::operation_aborted)
            {
                throw;
            }

            std::cerr
                << "[request handler] system error: "
                << error.what()
                << '\n';
        }
        catch (const std::exception &error)
        {
            std::cerr
                << "[request handler] exception: "
                << error.what()
                << '\n';
        }
        catch (...)
        {
            std::cerr
                << "[request handler] unknown exception\n";
        }

        co_return makeInternalServerError(request_version);
    };
}

} // namespace

//------------------------------------------------------------------------------

int main(int argc, char *argv[])
{
    try
    {
        ProgramOptions config = parseProgramOptions(argc, argv);

        config.public_dir = resolvePublicDirectory(config);

        net::io_context io_context;

        tasks::TaskRepository repository(config.database_file);

        auto scheduler = std::make_shared<tasks::TaskScheduler>(
            io_context.get_executor(),
            repository);

        auto task_service = std::make_shared<tasks::TaskService>(
            repository,
            scheduler);

        auto task_api_handler =
            std::make_shared<application::TaskApiHandler>(task_service);

        auto static_file_handler =
            std::make_shared<server::StaticFileHandler>(config.public_dir);

        auto application_handler =
            [task_api_handler, static_file_handler](
                server::HttpRequest &&request) mutable
            -> server::AwaitableResponse {
            constexpr std::string_view kApiPrefix = "/api/";

            if (request.target.starts_with(kApiPrefix))
            {
                co_return co_await task_api_handler->handle(
                    std::move(request));
            }

            co_return static_file_handler->handle(
                std::move(request));
        };

        auto request_handler_with_exception =
            withExceptionHandling(std::move(application_handler));

        const auto web_server = createWebServer(
            config,
            io_context,
            request_handler_with_exception);

        net::signal_set signals(io_context, SIGINT, SIGTERM);

        setupSignalHandling(signals, io_context, web_server);

        restoreScheduledTasks(io_context, scheduler);

        runWebServer(config, io_context, web_server);

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