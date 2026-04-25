#pragma once

#include "GraphicPrimitive.hpp"

#include <iostream>
#include <string>

class Line final : public GraphicPrimitive
{
public:
    Line(Point start, Point end)
        : start_(start)
        , end_(end)
    {
    }

    void draw() const override
    {
        std::cout << "\tDraw line: from x=" << start_.x
                  << ", y=" << start_.y
                  << " to x=" << end_.x
                  << ", y=" << end_.y
                  << '\n';
    }

    std::string type() const override
    {
        return "Line";
    }

private:
    Point start_{};
    Point end_{};
};