#include "TaskApiHandler.hpp"

#include "tasks/TaskService.hpp"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <charconv>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

namespace net = boost::asio;
namespace pt  = boost::property_tree;

namespace application
{

using tasks::Task;
using tasks::TaskId;
using tasks::TaskInput;
using tasks::TaskNotFoundError;
using tasks::toString;

namespace
{

struct ParsedRoute
{
    enum class Type
    {
        kCollection = 0,
        kItem,
        kRun,
        kUnknown
    };

    Type                  type = Type::kUnknown;
    std::optional<TaskId> id;
};

//------------------------------------------------------------------------------

std::string removeQueryString(std::string target)
{
    const std::string::size_type query_position = target.find('?');

    if (query_position != std::string::npos)
    {
        target.erase(query_position);
    }

    while (target.size() > 1 && target.back() == '/')
    {
        target.pop_back();
    }

    return target;
}

//------------------------------------------------------------------------------

std::optional<TaskId> parseTaskId(std::string_view value)
{
    TaskId id = 0;

    const auto [pointer, error] = std::from_chars(
        value.data(),
        value.data() + value.size(),
        id);

    if (error != std::errc{} ||
        pointer != value.data() + value.size() ||
        id <= 0)
    {
        return std::nullopt;
    }

    return id;
}

//------------------------------------------------------------------------------

ParsedRoute parseRoute(const std::string &raw_target)
{
    const std::string target = removeQueryString(raw_target);

    if (target == "/api/tasks")
    {
        return ParsedRoute{ParsedRoute::Type::kCollection, std::nullopt};
    }

    constexpr std::string_view kPrefix = "/api/tasks/";

    if (!target.starts_with(kPrefix))
    {
        return {};
    }

    const std::string_view suffix{
        target.data() + kPrefix.size(),
        target.size() - kPrefix.size()};

    constexpr std::string_view kRunSuffix = "/run";

    if (suffix.ends_with(kRunSuffix))
    {
        const std::string_view id_text = suffix.substr(
            0,
            suffix.size() - kRunSuffix.size());

        const std::optional<TaskId> id = parseTaskId(id_text);

        if (!id.has_value())
        {
            return {};
        }

        return ParsedRoute{ParsedRoute::Type::kRun, id};
    }

    const std::optional<TaskId> id = parseTaskId(suffix);

    if (!id.has_value())
    {
        return {};
    }

    return ParsedRoute{ParsedRoute::Type::kItem, id};
}

//------------------------------------------------------------------------------

std::string escapeJsonString(const std::string &value)
{
    std::ostringstream result;

    for (const unsigned char character : value)
    {
        switch (character)
        {
            case '"':
                result << "\\\"";
                break;

            case '\\':
                result << "\\\\";
                break;

            case '\b':
                result << "\\b";
                break;

            case '\f':
                result << "\\f";
                break;

            case '\n':
                result << "\\n";
                break;

            case '\r':
                result << "\\r";
                break;

            case '\t':
                result << "\\t";
                break;

            default:
                if (character < 0x20)
                {
                    result << "\\u"
                           << std::hex
                           << std::setw(4)
                           << std::setfill('0')
                           << static_cast<unsigned>(character)
                           << std::dec;
                }
                else
                {
                    result << static_cast<char>(character);
                }
                break;
        }
    }

    return result.str();
}

//------------------------------------------------------------------------------

std::string quoteJson(const std::string &value)
{
    return "\"" + escapeJsonString(value) + "\"";
}

//------------------------------------------------------------------------------

std::string taskToJson(const Task &task)
{
    std::ostringstream result;

    result << '{'
           << "\"id\":" << task.id << ','
           << "\"title\":" << quoteJson(task.title) << ','
           << "\"description\":" << quoteJson(task.description) << ','
           << "\"scheduledAt\":" << task.scheduled_at_ms << ','
           << "\"status\":" << quoteJson(toString(task.status)) << ','
           << "\"errorMessage\":";

    if (task.error_message.has_value())
    {
        result << quoteJson(*task.error_message);
    }
    else
    {
        result << "null";
    }

    result << ','
           << "\"createdAt\":" << task.created_at_ms << ','
           << "\"updatedAt\":" << task.updated_at_ms
           << '}';

    return result.str();
}

//------------------------------------------------------------------------------

std::string tasksToJson(const std::vector<Task> &tasks)
{
    std::ostringstream result;

    result << '[';

    for (std::size_t index = 0; index < tasks.size(); ++index)
    {
        if (index != 0)
        {
            result << ',';
        }

        result << taskToJson(tasks[index]);
    }

    result << ']';

    return result.str();
}

//------------------------------------------------------------------------------

TaskInput parseTaskInput(const std::string &body)
{
    std::istringstream input_stream(body);
    pt::ptree json;

    pt::read_json(input_stream, json);

    TaskInput input;

    input.title           = json.get<std::string>("title");
    input.description     = json.get<std::string>("description", "");
    input.scheduled_at_ms = json.get<std::int64_t>("scheduledAt");

    return input;
}

//------------------------------------------------------------------------------

std::string makeErrorBody(const std::string &message)
{
    return "{\"error\":" + quoteJson(message) + '}';
}

//------------------------------------------------------------------------------

server::HttpResponse makeMethodNotAllowed(
    const server::HttpRequest &request,
    std::string                allowed_methods)
{
    server::HttpResponse response = server::HttpResponse::methodNotAllowed(
        makeErrorBody("Method Not Allowed"),
        request.keep_alive,
        request.version);

    response.headers.emplace_back(
        "Allow",
        std::move(allowed_methods));

    return response;
}

} // namespace

//------------------------------------------------------------------------------

TaskApiHandler::TaskApiHandler(
    std::shared_ptr<tasks::TaskService> service)
    : service_(std::move(service))
{
}

//------------------------------------------------------------------------------

net::awaitable<server::HttpResponse> TaskApiHandler::handle(
    server::HttpRequest request) const
{
    try
    {
        const ParsedRoute route = parseRoute(request.target);

        if (route.type == ParsedRoute::Type::kUnknown)
        {
            co_return server::HttpResponse::notFound(
                makeErrorBody("API route was not found"),
                request.keep_alive,
                request.version);
        }

        if (route.type == ParsedRoute::Type::kCollection)
        {
            if (request.method == "GET")
            {
                const std::vector<Task> tasks = co_await service_->findAll();

                co_return server::HttpResponse::ok(
                    tasksToJson(tasks),
                    "application/json; charset=utf-8",
                    request.keep_alive,
                    request.version);
            }

            if (request.method == "POST")
            {
                const TaskInput input = parseTaskInput(request.body);
                const Task task = co_await service_->create(input);

                co_return server::HttpResponse::created(
                    taskToJson(task),
                    "/api/tasks/" + std::to_string(task.id),
                    request.keep_alive,
                    request.version);
            }

            co_return makeMethodNotAllowed(request, "GET, POST");
        }

        if (route.type == ParsedRoute::Type::kItem)
        {
            const TaskId id = *route.id;

            if (request.method == "GET")
            {
                const Task task = co_await service_->findById(id);

                co_return server::HttpResponse::ok(
                    taskToJson(task),
                    "application/json; charset=utf-8",
                    request.keep_alive,
                    request.version);
            }

            if (request.method == "PUT")
            {
                const TaskInput input = parseTaskInput(request.body);
                const Task task = co_await service_->update(id, input);

                co_return server::HttpResponse::ok(
                    taskToJson(task),
                    "application/json; charset=utf-8",
                    request.keep_alive,
                    request.version);
            }

            if (request.method == "DELETE")
            {
                co_await service_->remove(id);

                co_return server::HttpResponse::noContent(
                    request.keep_alive,
                    request.version);
            }

            co_return makeMethodNotAllowed(request, "GET, PUT, DELETE");
        }

        if (route.type == ParsedRoute::Type::kRun)
        {
            if (request.method != "POST")
            {
                co_return makeMethodNotAllowed(request, "POST");
            }

            const Task task = co_await service_->runNow(*route.id);

            co_return server::HttpResponse::ok(
                taskToJson(task),
                "application/json; charset=utf-8",
                request.keep_alive,
                request.version);
        }

        co_return server::HttpResponse::notFound(
            makeErrorBody("API route was not found"),
            request.keep_alive,
            request.version);
    }
    catch (const TaskNotFoundError &error)
    {
        co_return server::HttpResponse::notFound(
            makeErrorBody(error.what()),
            request.keep_alive,
            request.version);
    }
    catch (const pt::json_parser::json_parser_error &error)
    {
        co_return server::HttpResponse::badRequest(
            makeErrorBody(error.what()),
            request.keep_alive,
            request.version);
    }
    catch (const pt::ptree_error &error)
    {
        co_return server::HttpResponse::badRequest(
            makeErrorBody(error.what()),
            request.keep_alive,
            request.version);
    }
    catch (const std::invalid_argument &error)
    {
        co_return server::HttpResponse::badRequest(
            makeErrorBody(error.what()),
            request.keep_alive,
            request.version);
    }
    catch (const std::exception &error)
    {
        co_return server::HttpResponse::internalServerError(
            makeErrorBody(error.what()),
            request.keep_alive,
            request.version);
    }
}

} // namespace application
