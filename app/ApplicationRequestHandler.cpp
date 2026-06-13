#include "ApplicationRequestHandler.hpp"

#include "TaskApiHandler.hpp"

#include <string_view>
#include <utility>

namespace net = boost::asio;

namespace application
{

//------------------------------------------------------------------------------

ApplicationRequestHandler::ApplicationRequestHandler(
    server::StaticFileHandler       static_file_handler,
    std::shared_ptr<TaskApiHandler> task_api_handler)
    : static_file_handler_(std::move(static_file_handler))
    , task_api_handler_(std::move(task_api_handler))
{
}

//------------------------------------------------------------------------------

net::awaitable<server::HttpResponse> ApplicationRequestHandler::handle(
    server::HttpRequest request)
{
    constexpr std::string_view kApiPrefix = "/api/";

    if (request.target.starts_with(kApiPrefix))
    {
        co_return co_await task_api_handler_->handle(std::move(request));
    }

    co_return static_file_handler_.handle(std::move(request));
}

//------------------------------------------------------------------------------

} // namespace application
