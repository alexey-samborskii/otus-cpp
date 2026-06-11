#include "server/HttpRequestHandler.hpp"

#include "server/Router.hpp"

#include <utility>

namespace server
{

HttpResponse handleHttpRequest(
    const std::filesystem::path &public_dir,
    HttpRequest                &&request)
{
    Router router(
        public_dir);

    return router.route(
        std::move(request));
}

} // namespace server