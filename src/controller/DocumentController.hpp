#pragma once

#include "../model/Document.hpp"
#include "../model/GraphicPrimitive.hpp"

#include <memory>

class DocumentController
{
public:
    explicit DocumentController(std::shared_ptr<Document> document)
        : document_(std::move(document))
    {
    }

    void createNewDocument()
    {
        document_->clear();
    }

    void addPrimitive(std::unique_ptr<GraphicPrimitive> primitive)
    {
        document_->addPrimitive(std::move(primitive));
    }

    void removePrimitive(std::size_t index)
    {
        document_->removePrimitive(index);
    }

private:
    std::shared_ptr<Document> document_;
};