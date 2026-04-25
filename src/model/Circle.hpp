#pragma once

#include "GraphicPrimitive.hpp"

#include <iostream>
#include <string>

class Circle final : public GraphicPrimitive
{
public:
    Circle(Point center, double radius)
        : center_(center)
        , radius_(radius)
    {
    }

    void draw() const override
    {
        std::cout << "\tDraw circle: x=" << center_.x
                  << ", y=" << center_.y
                  << ", radius=" << radius_
                  << '\n';
    }

    std::string type() const override
    {
        return "Circle";
    }

private:
    Point center_{};
    double radius_{};
};