#pragma once

#include "GraphicPrimitive.hpp"

#include <iostream>
#include <string>

class Rectangle final : public GraphicPrimitive
{
public:
    Rectangle(Point position, double width, double height)
        : position_(position)
        , width_(width)
        , height_(height)
    {
    }

    void draw() const override
    {
        std::cout << "\tDraw rectangle: x=" << position_.x
                  << ", y=" << position_.y
                  << ", width=" << width_
                  << ", height=" << height_
                  << '\n';
    }

    std::string type() const override
    {
        return "Rectangle";
    }

private:
    Point position_{};
    double width_{};
    double height_{};
};