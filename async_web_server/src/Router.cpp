#include "server/Router.hpp"

#include <utility>

namespace server
{

Router::Router(
    std::filesystem::path public_dir)
    : static_file_handler_(std::move(public_dir))
{
}

//------------------------------------------------------------------------------

HttpResponse Router::route(
    HttpRequest &&request) const
{
    return static_file_handler_.handle(
        std::move(request));
}

} // namespace server