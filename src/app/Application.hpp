#pragma once

#include "../controller/DocumentController.hpp"
#include "../factory/PrimitiveFactory.hpp"
#include "../io/FileManager.hpp"
#include "../model/Document.hpp"
#include "../view/DocumentView.hpp"

#include <filesystem>
#include <iostream>
#include <memory>

class Application
{
public:
    Application()
        : document_(std::make_shared<Document>())
        , controller_(document_)
        , view_(document_)
    {
    }

    void createNewDocument()
    {
        controller_.createNewDocument();
    }

    void importDocument(const std::filesystem::path& file_path)
    {
        FileManager::importDocument(file_path, *document_);
    }

    void exportDocument(const std::filesystem::path& file_path) const
    {
        std::cout << "[Application] Export document to: " << file_path << '\n';

        FileManager::exportDocument(file_path, *document_);

        std::cout << "[Application] Current document content:\n";
        view_.render();
    }

    void createRectangle()
    {
        controller_.addPrimitive(PrimitiveFactory::createRectangle());
        view_.render();
    }

    void createCircle()
    {
        controller_.addPrimitive(PrimitiveFactory::createCircle());
        view_.render();
    }

    void createLine()
    {
        controller_.addPrimitive(PrimitiveFactory::createLine());
        view_.render();
    }

    void deletePrimitive(std::size_t index)
    {
        controller_.removePrimitive(index);
        view_.render();
    }

    void render() const
    {
        view_.render();
    }

private:
    std::shared_ptr<Document> document_;
    DocumentController       controller_;
    DocumentView             view_;
};