#pragma once

#include "../model/Circle.hpp"
#include "../model/GraphicPrimitive.hpp"
#include "../model/Line.hpp"
#include "../model/Rectangle.hpp"

#include <memory>

class PrimitiveFactory
{
public:
    static std::unique_ptr<GraphicPrimitive> createRectangle()
    {
        return std::make_unique<Rectangle>(Point{10.0, 20.0}, 100.0, 50.0);
    }

    static std::unique_ptr<GraphicPrimitive> createCircle()
    {
        return std::make_unique<Circle>(Point{50.0, 50.0}, 25.0);
    }

    static std::unique_ptr<GraphicPrimitive> createLine()
    {
        return std::make_unique<Line>(Point{0.0, 0.0}, Point{100.0, 100.0});
    }
};