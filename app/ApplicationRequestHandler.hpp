#pragma once

#include "server/HttpRequestHandler.hpp"
#include "server/StaticFileHandler.hpp"

#include <memory>

namespace application
{

class TaskApiHandler;

class ApplicationRequestHandler final : public server::HttpRequestHandler
{
public:
    ApplicationRequestHandler(
        server::StaticFileHandler       static_file_handler,
        std::shared_ptr<TaskApiHandler> task_api_handler);

    boost::asio::awaitable<server::HttpResponse> handle(
        server::HttpRequest request) override;

private:
    server::StaticFileHandler       static_file_handler_;
    std::shared_ptr<TaskApiHandler> task_api_handler_;
};

} // namespace application
