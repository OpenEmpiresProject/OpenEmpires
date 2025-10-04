#ifndef DRSCOLOR_H
#define DRSCOLOR_H

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
} // namespace drs

#endif