#pragma once

#include "../model/Circle.hpp"
#include "../model/Document.hpp"
#include "../model/Line.hpp"
#include "../model/Rectangle.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>

class FileManager
{
public:
    static bool importDocument(const std::filesystem::path& file_path, Document& document)
    {
        const auto content = readFile(file_path);
        if (content.empty())
        {
            return false;
        }

        std::istringstream input(content);
        document.clear();

        std::string type;
        while (input >> type)
        {
            if (type == "RECTANGLE")
            {
                double x{}, y{}, width{}, height{};
                input >> x >> y >> width >> height;
                document.addPrimitive(std::make_unique<Rectangle>(Point{x, y}, width, height));
            }
            else if (type == "CIRCLE")
            {
                double x{}, y{}, radius{};
                input >> x >> y >> radius;
                document.addPrimitive(std::make_unique<Circle>(Point{x, y}, radius));
            }
            else if (type == "LINE")
            {
                double x1{}, y1{}, x2{}, y2{};
                input >> x1 >> y1 >> x2 >> y2;
                document.addPrimitive(std::make_unique<Line>(Point{x1, y1}, Point{x2, y2}));
            }
        }

        std::cout << "[FileManager] Import document from: " << file_path << '\n';
        return true;
    }

    static bool exportDocument(const std::filesystem::path& file_path, const Document& document)
    {
        std::ostringstream output;

        for (const auto& primitive : document.primitives())
        {
            output << primitive->serialize() << '\n';
        }

        const bool ok = writeFile(file_path, output.str());

        if (ok)
        {
            std::cout << "[FileManager] Export document to: " << file_path << '\n';
        }

        return ok;
    }

private:
    static std::string readFile(const std::filesystem::path& file_path)
    {
        std::ifstream file(file_path);
        if (!file)
        {
            std::cerr << "[FileManager] Failed to open file for reading: "
                      << file_path << '\n';
            return {};
        }

        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static bool writeFile(const std::filesystem::path& file_path, const std::string& content)
    {
        std::ofstream file(file_path);
        if (!file)
        {
            std::cerr << "[FileManager] Failed to open file for writing: "
                      << file_path << '\n';
            return false;
        }

        file << content;
        return true;
    }
};