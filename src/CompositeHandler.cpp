#include "CompositeHandler.hpp"

namespace bulk
{

void CompositeHandler::add(std::shared_ptr<ICommandBlockHandler> handler)
{
    if (handler)
    {
        handlers_.push_back(handler);
    }
}

//------------------------------------------------------------------------------

void CompositeHandler::handle(const CommandBlock& bulk)
{
    for (const auto& handler : handlers_)
    {
        handler->handle(bulk);
    }
}

} // namespace bulk