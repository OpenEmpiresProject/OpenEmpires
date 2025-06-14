#ifndef COLOR_H
#define COLOR_H

#include <cstdint>

namespace ion
{
struct Color
{
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;
    uint8_t a = 255;

    constexpr Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b)
    {
    }
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a)
    {
    }

    bool operator==(const Color& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    bool operator!=(const Color& other) const
    {
        return !(*this == other);
    }

    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color PURPLE;
    static const Color YELLOW;
    static const Color NONE;
    static const Color BLACK;
    static const Color WHITE;
    static const Color GREY;
};
} // namespace ion

#endif