#ifndef COLOR_H
#define COLOR_H

#include <cstdint>

namespace core
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

    constexpr bool operator==(const Color& other) const
    {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }

    constexpr bool operator!=(const Color& other) const
    {
        return !(*this == other);
    }

    Color withAlpha20() const
    {
        return {r, g, b, (uint8_t) (255 * 0.2)};
    }

    Color withAlpha40() const
    {
        return {r, g, b, (uint8_t) (255 * 0.4)};
    }

    Color withAlpa50() const
    {
        return {r, g, b, 128};
    }

    Color withAlpha60() const
    {
        return {r, g, b, (uint8_t) (255 * 0.6)};
    }

    Color withAlpha80() const
    {
        return {r, g, b, (uint8_t) (255 * 0.8)};
    }

    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color PURPLE;
    static const Color YELLOW;
    static const Color NONE;
    static const Color WHITE;
    static const Color BLACK;
    static const Color GREY;
    static const Color CYAN;
    static const Color MAGENTA;
    static const Color ORANGE;
    static const Color PINK;
    static const Color BROWN;
    static const Color LIME;
    static const Color OLIVE;
    static const Color TEAL;
    static const Color NAVY;
    static const Color MAROON;
    static const Color VIOLET;
    static const Color SILVER;
    static const Color DARK_GREY;
    static const Color LIGHT_GREY;
};
} // namespace core

#endif