#pragma once

#include "server/HttpRequest.hpp"
#include "server/HttpResponse.hpp"

#include <filesystem>

namespace server
{

HttpResponse handleHttpRequest(
    const std::filesystem::path &public_dir,
    HttpRequest                &&request);

} // namespace server