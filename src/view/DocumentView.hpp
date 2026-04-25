#pragma once

#include "../model/Document.hpp"

class DocumentView
{
public:
    explicit DocumentView(const Document& document)
        : document_(document)
    {
    }

    void render() const
    {
        for (const auto& primitive : document_.primitives())
        {
            primitive->draw();
        }
    }

private:
    const Document& document_;
};