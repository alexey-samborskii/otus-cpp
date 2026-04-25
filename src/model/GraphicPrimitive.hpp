#pragma once

#include <string>

struct Point
{
    double x{};
    double y{};
};

class GraphicPrimitive
{
public:
    virtual ~GraphicPrimitive() = default;

    virtual void        draw() const = 0;
    virtual std::string type() const = 0;
};