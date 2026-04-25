#pragma once

#include "GraphicPrimitive.hpp"

#include <iostream>
#include <string>
#include <sstream>

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

    std::string serialize() const override
    {
        std::ostringstream out;
        out << "CIRCLE " << center_.x << ' ' << center_.y << ' ' << radius_;
        return out.str();
    }

private:
    Point  center_{};
    double radius_{};
};