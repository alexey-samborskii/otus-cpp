#pragma once

#include <string>

namespace server
{

struct HttpRequest
{
    std::string method;
    std::string target;
    std::string body;
    std::string content_type;
    bool        keep_alive = false;
    unsigned    version    = 11;
};

} // namespace server
