#ifndef FRAME_H
#define FRAME_H

#include <span>
#include <cstdint>
#include <vector>

namespace drs
{
    
#pragma pack(push, 1)
    struct SLPFrameInfo {
    uint32_t cmdTableOffset;
    uint32_t outlineTableOffset;
    uint32_t paletteOffset;
    uint32_t properties;
    int32_t  width;
    int32_t  height;
    int32_t  hotspot_x;
    int32_t  hotspot_y;
    };
#pragma pack(pop)

    class Frame
    {
    public:
        void load(const SLPFrameInfo& fi, std::span<const uint8_t> data);

        std::vector<std::vector<uint32_t>> m_image;

        static inline const int MAX_IMAGE_SIZE = 500;
    };
} // namespace drs


#endif