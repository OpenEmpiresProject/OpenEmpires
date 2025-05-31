#ifndef PALETTES_H
#define PALETTES_H

#include "Color.h"

#include <array>

namespace drs
{
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