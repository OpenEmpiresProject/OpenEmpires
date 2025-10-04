#ifndef FRAMEINFO_H
#define FRAMEINFO_H

#include <cstdint>

namespace drs
{
#pragma pack(push, 1)
struct FrameInfo
{
    uint32_t cmdTableOffset;
    uint32_t outlineTableOffset;
    uint32_t paletteOffset;
    uint32_t properties;
    int32_t width;
    int32_t height;
    int32_t hotspot_x;
    int32_t hotspot_y;
};
#pragma pack(pop)
} // namespace drs

#endif