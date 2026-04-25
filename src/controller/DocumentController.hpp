#pragma once

#include "../model/Document.hpp"
#include "../model/GraphicPrimitive.hpp"

#include <memory>

class DocumentController
{
public:
    explicit DocumentController(Document& document)
        : document_(document)
    {
    }

    void createNewDocument()
    {
        document_.clear();
    }

    void addPrimitive(std::unique_ptr<GraphicPrimitive> primitive)
    {
        document_.addPrimitive(std::move(primitive));
    }

    void removePrimitive(std::size_t index)
    {
        document_.removePrimitive(index);
    }

private:
    Document& document_;
};