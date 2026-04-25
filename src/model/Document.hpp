#pragma once

#include "GraphicPrimitive.hpp"

#include <cstddef>
#include <memory>
#include <vector>

class Document
{
public:
    void addPrimitive(std::unique_ptr<GraphicPrimitive> primitive)
    {
        primitives_.push_back(std::move(primitive));
    }

    void removePrimitive(std::size_t index)
    {
        if (index < primitives_.size())
        {
            primitives_.erase(primitives_.begin() + static_cast<std::ptrdiff_t>(index));
        }
    }

    void clear()
    {
        primitives_.clear();
    }

    const std::vector<std::unique_ptr<GraphicPrimitive>>& primitives() const
    {
        return primitives_;
    }

private:
    std::vector<std::unique_ptr<GraphicPrimitive>> primitives_;
};