#pragma once

#include "ICommandBlockHandler.hpp"

#include <string>

namespace bulk
{

class FileHandler : public ICommandBlockHandler
{
public:
    explicit FileHandler(std::string output_dir = ".");

    void handle(const CommandBlock& bulk) override;

private:
    std::string output_dir_;
};

} // namespace bulk