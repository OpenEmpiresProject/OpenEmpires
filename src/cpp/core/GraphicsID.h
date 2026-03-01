#ifndef GRAPHICS_ID_H
#define GRAPHICS_ID_H

#include "utils/Types.h"

#include <SDL3/SDL.h>
#include <fmt/core.h>
#include <string>

namespace core
{
class GraphicsID
{
  public:
    union
    {
        struct
        {
            uint64_t entityType : 15;    // 32,768 values
            uint64_t action : 10;        // 1,024 values
            uint64_t frame : 5;          // 32 values
            uint64_t direction : 4;      // 16 values
            uint64_t variation : 10;     // 1,024 values
            uint64_t playerId : 4;       // 16 values
            uint64_t civilization : 6;   // 64 values
            uint64_t age : 3;            // 8 values
            uint64_t orientation : 4;    // 8 values
            uint64_t state : 4;          // 8 values
            uint64_t isConstructing : 1; // 2 values (a bool)
            uint64_t uiElementType : 9;  // 512 values
            uint64_t isShadow : 1;       // 2 values (a bool)
            uint64_t isIcon : 1;         // 2 values (a bool)
            uint64_t reserved : 46;
        };
        struct
        {
            uint64_t low;
            uint64_t high;
        } raw;
    };

    GraphicsID(uint64_t type) : GraphicsID()
    {
        entityType = type;
    }

    GraphicsID()
    {
        raw.low = 0;
        raw.high = 0;
        entityType = 0;
        action = 0;
        frame = 0;
        variation = 0;
        playerId = 0;
        civilization = 0;
        age = 0;
        state = 0;
        isConstructing = int(false);
        uiElementType = 0;
        isShadow = 0;
        isIcon = 0;
        direction = static_cast<uint64_t>(Direction::NONE);
        orientation = static_cast<uint64_t>(BuildingOrientation::NO_ORIENTATION);
        reserved = 0;
    }

    GraphicsID getBaseId() const
    {
        GraphicsID id;
        id.entityType = entityType;
        id.action = action;
        id.playerId = playerId;
        id.civilization = civilization;
        id.age = age;
        id.orientation = orientation;
        id.state = state;
        id.isConstructing = isConstructing;
        id.uiElementType = uiElementType;
        id.isShadow = isShadow;
        id.isIcon = isIcon;
        return id;
    }

    bool isValid() const
    {
        return entityType != 0;
    }

    bool operator==(const GraphicsID& o) const
    {
        return raw.low == o.raw.low && raw.high == o.raw.high;
    }

    bool operator!=(const GraphicsID& o) const
    {
        return !(*this == o);
    }

    bool operator<(const GraphicsID& other) const noexcept
    {
        return (raw.high < other.raw.high) ||
               (raw.high == other.raw.high && raw.low < other.raw.low);
    }

    std::string toShortString() const
    {
        return fmt::format(
            "GraphicsID(T{}{}{}{}{}{}{}{}{}{}{}{}{}{})", entityType,
            (action ? ", A" + std::to_string(action) : ""),
            (frame ? ", F" + std::to_string(frame) : ""),
            (direction != static_cast<uint64_t>(Direction::NONE)
                 ? ", D" + std::to_string(static_cast<int>(direction))
                 : ""),
            (variation ? ", V" + std::to_string(variation) : ""),
            (playerId ? ", P" + std::to_string(playerId) : ""),
            (civilization ? ", Civ" + std::to_string(civilization) : ""),
            (age ? ", Age" + std::to_string(age) : ""),
            (orientation != static_cast<uint64_t>(BuildingOrientation::NO_ORIENTATION)
                 ? ", O" + std::to_string(orientation)
                 : ""),
            (state ? ", S" + std::to_string(state) : ""),
            (isConstructing ? ", isC" + std::to_string(isConstructing) : ""),
            (uiElementType ? ", ui" + std::to_string(uiElementType) : ""),
            (isShadow ? ", isS" + std::to_string(isShadow) : ""),
            (isIcon ? ", isI" + std::to_string(isIcon) : ""));
    }

    std::string toString() const
    {
        return "GraphicsID(T" + std::to_string(entityType) + ", A" + std::to_string(action) +
               ", F" + std::to_string(frame) + ", D" + std::to_string(static_cast<int>(direction)) +
               ", V" + std::to_string(variation) + ", P" + std::to_string(playerId) + ", Civ" +
               std::to_string(civilization) + ", Age" + std::to_string(age) + ", O" +
               std::to_string(orientation) + ", S" + std::to_string(state) + ", isC" +
               std::to_string(isConstructing) + ", ui" + std::to_string(uiElementType) + ", isS" +
               std::to_string(isShadow) + ", isI" + std::to_string(isIcon) + ")";
    }

  private:
    static inline uint64_t rotl(uint64_t v, unsigned int k) noexcept
    {
        return (v << k) | (v >> (64 - k));
    }
};

static_assert(sizeof(GraphicsID) == 16, "GraphicsID size should be limited to 128bits");

} // namespace core

// Hash functor for unordered_map
namespace std
{
template <> struct hash<core::GraphicsID>
{
    size_t operator()(const core::GraphicsID& id) const noexcept
    {
        // Simple 64-bit mix
        uint64_t low = id.raw.low;
        uint64_t high = id.raw.high;
        // SplitMix64-ish mixing
        low ^= high + 0x9e3779b97f4a7c15ULL + (low << 6) + (low >> 2);
        return static_cast<size_t>(low);
    }
};
} // namespace std

#endif