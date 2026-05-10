#include "async.h"

#include "AsyncContext.hpp"

#include <memory>

namespace async
{

//------------------------------------------------------------------------------

handle_t connect(std::size_t bulk)
{
    return new ::bulk::AsyncContext(bulk);
}

//------------------------------------------------------------------------------

void receive(handle_t handle, const char* data, std::size_t size)
{
    if (handle == nullptr)
    {
        return;
    }

    auto* context = static_cast<::bulk::AsyncContext*>(handle);
    context->receive(data, size);
}

//------------------------------------------------------------------------------

void disconnect(handle_t handle)
{
    if (handle == nullptr)
    {
        return;
    }

    std::unique_ptr<::bulk::AsyncContext> context(
        static_cast<::bulk::AsyncContext*>(handle));

    context->disconnect();
}

//------------------------------------------------------------------------------

} // namespace async