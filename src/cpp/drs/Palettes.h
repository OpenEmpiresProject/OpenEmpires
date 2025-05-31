#ifndef PALETTES_H
#define PALETTES_H

#include <array>
#include <cstdint>

namespace drs
{
struct Color
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    bool operator==(const Color& other) const
    {
        return r == other.r && g == other.g && b == other.b;
    }

    bool operator!=(const Color& other) const
    {
        return !(*this == other);
    }
};

struct Palette
{
    std::array<Color, 256> colors;
};

struct PaletteCollection
{
    static const std::array<Palette, 1> palettes;
};
} // namespace drs

#endif