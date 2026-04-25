#pragma once

#include "../model/Document.hpp"

#include <filesystem>
#include <string>
#include <fstream>

class FileManager
{
public:
    static bool importDocument(const std::filesystem::path& file_path, Document& document)
    {
        std::ifstream file(file_path);

        if (!file.is_open())
        {
            std::cerr << "[FileManager] Failed to open file for import: "
                      << file_path << '\n';
            return false;
        }

        document.clear();

        std::string type;

        while (file >> type)
        {
            if (type == "RECTANGLE")
            {
                double x{};
                double y{};
                double width{};
                double height{};

                file >> x >> y >> width >> height;

                document.addPrimitive(
                    std::make_unique<Rectangle>(
                        Point{x, y},
                        width,
                        height));
            }
            else if (type == "CIRCLE")
            {
                double x{};
                double y{};
                double radius{};

                file >> x >> y >> radius;

                document.addPrimitive(
                    std::make_unique<Circle>(
                        Point{x, y},
                        radius));
            }
            else if (type == "LINE")
            {
                double x1{};
                double y1{};
                double x2{};
                double y2{};

                file >> x1 >> y1 >> x2 >> y2;

                document.addPrimitive(
                    std::make_unique<Line>(
                        Point{x1, y1},
                        Point{x2, y2}));
            }
            else
            {
                std::cerr << "[FileManager] Unknown primitive type: "
                          << type << '\n';
                return false;
            }
        }

        std::cout << "[FileManager] Import document from: "
                  << file_path << '\n';

        return true;
    }

    static bool exportDocument(const std::filesystem::path& file_path, const Document& document)
    {
        std::ofstream file(file_path);

        if (!file.is_open())
        {
            std::cerr << "[FileManager] Failed to open file for export: "
                      << file_path << '\n';
            return false;
        }

        for (const auto& primitive : document.primitives())
        {
            if (primitive->type() == "Rectangle")
            {
                file << "RECTANGLE 10 20 100 50\n";
            }
            else if (primitive->type() == "Circle")
            {
                file << "CIRCLE 50 50 25\n";
            }
            else if (primitive->type() == "Line")
            {
                file << "LINE 0 0 100 100\n";
            }
        }

        std::cout << "[FileManager] Export document to: "
                  << file_path << '\n';

        return true;
    }

private:
    static std::string readFile(const std::filesystem::path& file_path)
    {
        (void)file_path;

        return {};
    }

    static bool writeFile(const std::filesystem::path& file_path, const std::string& content)
    {
        (void)file_path;
        (void)content;

        return true;
    }
};