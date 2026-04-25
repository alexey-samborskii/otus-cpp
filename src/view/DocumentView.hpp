#pragma once

#include "../model/Document.hpp"

#include <memory>

class DocumentView
{
public:
    explicit DocumentView(std::shared_ptr<const Document> document)
        : document_(std::move(document))
    {
    }

    void render() const
    {
        for (const auto& primitive : document_->primitives())
        {
            primitive->draw();
        }
    }

private:
    std::shared_ptr<const Document> document_;
};