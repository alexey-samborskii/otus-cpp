#pragma once

#include "ICommandBlockHandler.hpp"

#include <memory>
#include <vector>

namespace bulk
{

class CompositeHandler : public ICommandBlockHandler
{
public:
    void add(std::shared_ptr<ICommandBlockHandler> handler);
    void handle(const CommandBlock& bulk) override;

private:
    std::vector<std::shared_ptr<ICommandBlockHandler>> handlers_;
};

} // namespace bulk