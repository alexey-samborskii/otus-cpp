#include "AppConfig.hpp"
#include "ApplicationMetrics.hpp"
#include "TaskApiHandler.hpp"

#include "common/Logger.hpp"
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
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <algorithm>
#include <cctype>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <system_error>
#include <thread>
#include <utility>
#include <vector>

namespace net = boost::asio;
namespace pt  = boost::property_tree;
namespace ssl = boost::asio::ssl;

namespace
{

using WebServerPtr          = std::shared_ptr<server::WebServer>;
using TaskSchedulerPtr      = std::shared_ptr<tasks::TaskScheduler>;
using TaskServicePtr        = std::shared_ptr<tasks::TaskService>;
using ApplicationMetricsPtr = std::shared_ptr<application::ApplicationMetrics>;

enum class ServerProtocol
{
    kHttp  = 0,
    kHttps = 1
};

struct ProgramOptions
{
    ServerProtocol        protocol         = ServerProtocol::kHttps;
    std::string           host             = "0.0.0.0";
    std::uint16_t         port             = 8443;
    bool                  port_set         = false;
    std::size_t           threads          = 4;
    std::filesystem::path database_file    = "data/tasks.db";
    std::filesystem::path certificate_file = "certs/server.crt";
    std::filesystem::path private_key_file = "certs/server.key";
    std::string           log_level        = "info";
    bool                  public_dir_set   = false;
    std::filesystem::path public_dir;
    std::filesystem::path config_file;
};

//------------------------------------------------------------------------------

void printUsage(const char *program_name)
{
    std::cout
        << "Usage: " << program_name << " [--config <path>] [options]\n\n"
        << "Options:\n"
        << "  --config <path>         JSON configuration file\n"
        << "  --protocol <http|https> Server protocol, default: https\n"
        << "  --host <address>        Bind address, default: 0.0.0.0\n"
        << "  --port <number>         Bind port, default: 8080/8443\n"
        << "  --threads <number>      io_context worker threads, default: 4\n"
        << "  --public-dir <path>     Web interface directory\n"
        << "                          Default: automatic detection\n"
        << "  --database <path>       SQLite database file\n"
        << "  --cert <path>           TLS certificate file\n"
        << "  --key <path>            TLS private key file\n"
        << "  --log-level <level>     debug, info, warning, error\n"
        << "  --help                  Show this help\n";
}

//------------------------------------------------------------------------------

std::string requireValue(int argc, char *argv[], int &index, const char *name)
{
    if (index + 1 >= argc)
    {
        throw std::invalid_argument(std::string("Missing value for ") + name);
    }

    ++index;

    return argv[index];
}

//------------------------------------------------------------------------------

std::string normalizeConfigKey(std::string value)
{
    std::replace(value.begin(), value.end(), '-', '_');

    return value;
}

//------------------------------------------------------------------------------

bool containsOnlyDigits(const std::string &value)
{
    return !value.empty() &&
           std::all_of(
               value.begin(),
               value.end(),
               [](unsigned char character) {
                   return std::isdigit(character);
               });
}

//------------------------------------------------------------------------------

auto parsePort(const std::string &value) -> std::uint16_t
{
    if (!containsOnlyDigits(value))
    {
        throw std::invalid_argument("Port must contain only digits");
    }

    try
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
    catch (const std::out_of_range &)
    {
        throw std::invalid_argument("Port must be in range 1..65535");
    }
}

//------------------------------------------------------------------------------

auto parseThreadCount(const std::string &value) -> std::size_t
{
    if (!containsOnlyDigits(value))
    {
        throw std::invalid_argument("Thread count must contain only digits");
    }

    try
    {
        std::size_t parsed_characters = 0;

        const unsigned long parsed = std::stoul(value, &parsed_characters);

        if (parsed_characters != value.size())
        {
            throw std::invalid_argument("Thread count must contain only digits");
        }

        if (parsed == 0)
        {
            throw std::invalid_argument("Thread count must be greater zero");
        }

        if (parsed > std::numeric_limits<std::size_t>::max())
        {
            throw std::invalid_argument("Thread count is too large");
        }

        return static_cast<std::size_t>(parsed);
    }
    catch (const std::out_of_range &)
    {
        throw std::invalid_argument("Thread count is too large");
    }
}

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

    throw std::invalid_argument("Protocol must be either http or https");
}

//------------------------------------------------------------------------------

void applyConfigValue(
    ProgramOptions    &config,
    const std::string &raw_key,
    const std::string &value)
{
    const std::string key = normalizeConfigKey(raw_key);

    if (key == "protocol" || key == "server.protocol")
    {
        config.protocol = parseProtocol(value);
        return;
    }

    if (key == "host" || key == "server.host")
    {
        config.host = value;
        return;
    }

    if (key == "port" || key == "server.port")
    {
        config.port     = parsePort(value);
        config.port_set = true;
        return;
    }

    if (key == "threads" || key == "server.threads")
    {
        config.threads = parseThreadCount(value);
        return;
    }

    if (key == "public_dir" || key == "server.public_dir")
    {
        config.public_dir     = value;
        config.public_dir_set = true;
        return;
    }

    if (key == "database" ||
        key == "database_file" ||
        key == "storage.database" ||
        key == "storage.database_file")
    {
        config.database_file = value;
        return;
    }

    if (key == "cert" ||
        key == "certificate_file" ||
        key == "tls.cert" ||
        key == "tls.certificate_file")
    {
        config.certificate_file = value;
        return;
    }

    if (key == "key" ||
        key == "private_key_file" ||
        key == "tls.key" ||
        key == "tls.private_key_file")
    {
        config.private_key_file = value;
        return;
    }

    if (key == "log_level" || key == "logging.level")
    {
        config.log_level = value;
        return;
    }
}

//------------------------------------------------------------------------------

void applyJsonConfig(ProgramOptions &config, const std::filesystem::path &path)
{
    pt::ptree tree;

    pt::read_json(path.string(), tree);

    const std::vector<std::pair<std::string, std::string>> keys = {
        {             "protocol",              "protocol"},
        {                 "host",                  "host"},
        {                 "port",                  "port"},
        {              "threads",               "threads"},
        {           "public_dir",            "public_dir"},
        {             "database",              "database"},
        {        "database_file",         "database_file"},
        {                 "cert",                  "cert"},
        {     "certificate_file",      "certificate_file"},
        {                  "key",                   "key"},
        {     "private_key_file",      "private_key_file"},
        {            "log_level",             "log_level"},
        {      "server.protocol",       "server.protocol"},
        {          "server.host",           "server.host"},
        {          "server.port",           "server.port"},
        {       "server.threads",        "server.threads"},
        {    "server.public_dir",     "server.public_dir"},
        {     "storage.database",      "storage.database"},
        {"storage.database_file", "storage.database_file"},
        {             "tls.cert",              "tls.cert"},
        { "tls.certificate_file",  "tls.certificate_file"},
        {              "tls.key",               "tls.key"},
        { "tls.private_key_file",  "tls.private_key_file"},
        {        "logging.level",         "logging.level"}
    };

    for (const auto &[path_name, config_key] : keys)
    {
        const auto value = tree.get_optional<std::string>(path_name);

        if (value.has_value())
        {
            applyConfigValue(config, config_key, *value);
        }
    }
}

//------------------------------------------------------------------------------

void applyConfigFile(
    ProgramOptions              &config,
    const std::filesystem::path &path)
{
    if (path.extension() != ".json")
    {
        throw std::invalid_argument("Config file extension must be .json");
    }

    applyJsonConfig(config, path);
}

//------------------------------------------------------------------------------

auto executablePath() -> std::filesystem::path
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

auto defaultConfigPaths() -> std::vector<std::filesystem::path>
{
    std::vector<std::filesystem::path> result;

    const std::filesystem::path executable_path = executablePath();

    if (!executable_path.empty())
    {
        const std::filesystem::path executable_directory =
            executable_path.parent_path();

        result.emplace_back(
            executable_directory /
            "config" /
            "async_task_web_server.json");
    }

    result.emplace_back(
        "/etc/async_task_web_server/async_task_web_server.json");

    return result;
}

//------------------------------------------------------------------------------

auto findExistingDefaultConfigPath()
    -> std::optional<std::filesystem::path>
{
    for (const std::filesystem::path &path : defaultConfigPaths())
    {
        std::error_code error;

        if (std::filesystem::is_regular_file(path, error) && !error)
        {
            return path;
        }
    }

    return std::nullopt;
}

//------------------------------------------------------------------------------

auto findConfigPath(int argc, char *argv[])
    -> std::optional<std::filesystem::path>
{
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (argument == "--config")
        {
            return requireValue(argc, argv, index, "--config");
        }
    }

    if (argc > 1)
    {
        const std::string first_argument = argv[1];

        if (!first_argument.starts_with("--"))
        {
            return first_argument;
        }
    }

    return findExistingDefaultConfigPath();
}

//------------------------------------------------------------------------------

auto localPublicDirectory() -> std::filesystem::path
{
    const std::filesystem::path executable_path = executablePath();

    if (executable_path.empty())
    {
        return {};
    }

    return executable_path.parent_path() / "public";
}

//------------------------------------------------------------------------------

auto installedPublicDirectory() -> std::filesystem::path
{
    const std::filesystem::path executable_path = executablePath();

    if (executable_path.empty())
    {
        return {};
    }

    std::filesystem::path install_prefix = executable_path.parent_path();

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

    return install_prefix / application::config::kInstallPublicDirectory;
}

//------------------------------------------------------------------------------

bool isUsablePublicDirectory(const std::filesystem::path &directory)
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

    std::ifstream index_file(directory / "index.html", std::ios::binary);

    return index_file.good();
}

//------------------------------------------------------------------------------

auto normalizePath(const std::filesystem::path &path) -> std::filesystem::path
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

auto resolvePublicDirectory(const ProgramOptions &config)
    -> std::filesystem::path
{
    if (config.public_dir_set)
    {
        if (isUsablePublicDirectory(config.public_dir))
        {
            return normalizePath(config.public_dir);
        }

        std::ostringstream message;

        message << "Public directory specified by --public-dir is unavailable: "
                << config.public_dir
                << ". Trying fallback directories";

        common::logWarning(message.str());
    }

    const std::filesystem::path local_directory = localPublicDirectory();

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

    std::string message = "Unable to locate a readable public directory";

    if (config.public_dir_set)
    {
        message += "\nRequested directory: " + config.public_dir.string();
    }

    if (!local_directory.empty())
    {
        message += "\nLocal directory: " + local_directory.string();
    }

    if (!installed_directory.empty())
    {
        message += "\nInstalled directory: " + installed_directory.string();
    }

    throw std::runtime_error(message);
}

//------------------------------------------------------------------------------

auto parseProgramOptions(int argc, char *argv[]) -> ProgramOptions
{
    ProgramOptions config;

    for (int index = 1; index < argc; ++index)
    {
        if (std::string(argv[index]) == "--help")
        {
            printUsage(argv[0]);
            std::exit(EXIT_SUCCESS);
        }
    }

    const std::optional<std::filesystem::path> config_path =
        findConfigPath(argc, argv);

    if (config_path.has_value())
    {
        config.config_file = *config_path;
        applyConfigFile(config, *config_path);
    }

    for (int index = 1; index < argc; ++index)
    {
        const std::string argument = argv[index];

        if (index == 1 && !argument.starts_with("--"))
        {
            continue;
        }

        if (argument == "--help")
        {
            continue;
        }

        if (argument == "--config")
        {
            requireValue(argc, argv, index, "--config");
            continue;
        }

        if (argument == "--protocol")
        {
            config.protocol = parseProtocol(
                requireValue(argc, argv, index, "--protocol"));
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
            config.threads = parseThreadCount(
                requireValue(argc, argv, index, "--threads"));
            continue;
        }

        if (argument == "--public-dir")
        {
            config.public_dir =
                requireValue(argc, argv, index, "--public-dir");
            config.public_dir_set = true;
            continue;
        }

        if (argument == "--database")
        {
            config.database_file = requireValue(argc, argv, index, "--database");
            continue;
        }

        if (argument == "--cert")
        {
            config.certificate_file = requireValue(argc, argv, index, "--cert");
            continue;
        }

        if (argument == "--key")
        {
            config.private_key_file = requireValue(argc, argv, index, "--key");
            continue;
        }

        if (argument == "--log-level")
        {
            config.log_level = requireValue(argc, argv, index, "--log-level");
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

auto stripQueryString(std::string target) -> std::string
{
    const std::string::size_type query_position = target.find('?');

    if (query_position != std::string::npos)
    {
        target.erase(query_position);
    }

    return target;
}

//------------------------------------------------------------------------------

auto createWebServer(
    const ProgramOptions         &config,
    net::io_context              &io_context,
    server::CallbackHandleRequest request_handler)
    -> WebServerPtr
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

auto setupShutdownSignalHandling(
    net::io_context    &io_context,
    const WebServerPtr &web_server)
    -> std::unique_ptr<net::signal_set>
{
    using error_code = boost::system::error_code;

    auto signals =
        std::make_unique<net::signal_set>(io_context, SIGINT, SIGTERM);

    signals->async_wait(
        [&io_context, web_server](const error_code &error, int signal_number) {
            if (error)
            {
                return;
            }

            common::logInfo("Received signal : " +
                            std::to_string(signal_number) + ", stop server");

            web_server->stop();
            io_context.stop();
        });

    return signals;
}

//------------------------------------------------------------------------------

void spawnRestoreScheduledTasks(
    net::io_context        &io_context,
    const TaskSchedulerPtr &scheduler)
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
                common::logError("[scheduler] restore error: " +
                                 std::string(exception.what()));
            }
            catch (...)
            {
                common::logError("[scheduler] restore unknown error");
            }
        });
}

//------------------------------------------------------------------------------

void spawnWebServerAcceptLoop(
    net::io_context    &io_context,
    const WebServerPtr &web_server)
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
                    common::logError("[server] acceptor error: " +
                                     std::string(exception.what()));
                }
                catch (...)
                {
                    common::logError("[server] acceptor unknown error");
                }
            }

            io_context.stop();
        });
}

//------------------------------------------------------------------------------

auto makeInternalServerError(unsigned int version) -> server::HttpResponse
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

auto makeHandlerWithException(
    server::CallbackHandleRequest request_handler,
    ApplicationMetricsPtr         metrics)
    -> server::CallbackHandleRequest
{
    return [request_handler = std::move(request_handler), metrics](
               server::HttpRequest &&request) mutable
               -> server::AwaitableResponse {
        const auto request_version = request.version;

        metrics->recordRequest();

        try
        {
            auto response = co_await request_handler(std::move(request));

            metrics->recordResponse(response.status);

            co_return response;
        }
        catch (const boost::system::system_error &exception)
        {
            if (exception.code() == net::error::operation_aborted)
            {
                throw;
            }

            metrics->recordException();
            common::logError("[request handler] system error: " +
                             std::string(exception.what()));
        }
        catch (const std::exception &exception)
        {
            metrics->recordException();
            common::logError("[request handler] system error: " +
                             std::string(exception.what()));
        }
        catch (...)
        {
            metrics->recordException();
            common::logError("[request handler] unknown exception");
        }

        metrics->recordResponse(500);

        co_return makeInternalServerError(request_version);
    };
}

//------------------------------------------------------------------------------

auto makeRequestHandler(
    const ProgramOptions   &config,
    const TaskSchedulerPtr &task_scheduler,
    const TaskServicePtr   &task_service)
    -> server::CallbackHandleRequest
{
    auto metrics =
        std::make_shared<application::ApplicationMetrics>();

    auto handler_api =
        std::make_shared<application::TaskApiHandler>(task_service);

    auto handler_static_file =
        std::make_shared<server::StaticFileHandler>(config.public_dir);

    auto handler_metrics_sync =
        [metrics, task_scheduler](
            server::HttpRequest &&request) -> server::HttpResponse {
        if (request.method != "GET")
        {
            return server::HttpResponse::methodNotAllowed(
                R"({"error":"Method Not Allowed"})",
                request.keep_alive,
                request.version);
        }

        return server::HttpResponse::ok(
            metrics->toPrometheusText(task_scheduler->metrics()),
            "text/plain; version=0.0.4; charset=utf-8",
            request.keep_alive,
            request.version);
    };

    auto request_handler =
        [request_handler_api          = std::move(handler_api),
         request_handler_static_file  = std::move(handler_static_file),
         request_handler_metrics_sync = std::move(handler_metrics_sync)](
            server::HttpRequest &&request) -> server::AwaitableResponse {
        const std::string target = stripQueryString(request.target);

        if (target == "/metrics")
        {
            co_return request_handler_metrics_sync(std::move(request));
        }

        if (target.starts_with("/api/"))
        {
            co_return co_await request_handler_api->handle(std::move(request));
        }

        co_return request_handler_static_file->handle(std::move(request));
    };

    return makeHandlerWithException(std::move(request_handler), metrics);
}

//------------------------------------------------------------------------------

auto makeConfiguration(int argc, char *argv[]) -> ProgramOptions
{
    auto config = parseProgramOptions(argc, argv);

    common::setLogLevel(common::logLevelFromString(config.log_level));

    if (!config.config_file.empty())
    {
        common::logInfo("Config file loaded: " + config.config_file.string());
    }

    config.public_dir = resolvePublicDirectory(config);

    return config;
}

//------------------------------------------------------------------------------

void run(const ProgramOptions &config, net::io_context &io_context)
{
    {
        std::ostringstream ss;

        const char *protocol =
            (config.protocol == ServerProtocol::kHttps ? "https" : "http");

        ss << "Server started:\n"
           << "  protocol         : " << protocol << '\n'
           << "  listen endpoint  : " << config.host << ':'
           << config.port << '\n'
           << "  public directory : " << config.public_dir << '\n'
           << "  SQLite database  : " << config.database_file << '\n'
           << "  worker threads   : " << config.threads;

        common::logInfo(ss.str());
    }

    std::vector<std::thread> workers;

    workers.reserve(config.threads - 1);

    for (std::size_t index = 1; index < config.threads; ++index)
    {
        workers.emplace_back([&io_context]() { io_context.run(); });
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
        auto config =
            makeConfiguration(argc, argv);

        auto io_context =
            net::io_context{};

        auto task_repository =
            tasks::TaskRepository{config.database_file};

        auto task_scheduler =
            std::make_shared<tasks::TaskScheduler>(
                io_context.get_executor(),
                task_repository);

        auto task_service =
            std::make_shared<tasks::TaskService>(
                task_repository,
                task_scheduler);

        auto request_handler =
            makeRequestHandler(config, task_scheduler, task_service);

        auto web_server =
            createWebServer(
                config,
                io_context,
                std::move(request_handler));

        [[maybe_unused]] auto shutdown_signal_handling =
            setupShutdownSignalHandling(io_context, web_server);

        spawnRestoreScheduledTasks(io_context, task_scheduler);

        spawnWebServerAcceptLoop(io_context, web_server);

        run(config, io_context);

        return EXIT_SUCCESS;
    }
    catch (const std::exception &exception)
    {
        common::logError("Fatal error: " + std::string(exception.what()));
    }
    catch (...)
    {
        common::logError("Fatal error: unknown exception.");
    }

    return EXIT_FAILURE;
}

//------------------------------------------------------------------------------