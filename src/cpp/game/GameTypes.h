#ifndef GAMETYPES_H
#define GAMETYPES_H

#include <cstdint>

namespace game
{
enum EntityTypes
{
    ET_UNKNOWN = 0,

    ET_TILE = 2,
    ET_VILLAGER = 3,
    ET_TREE = 4,

    ET_UI_ELEMENT = 10000
};

enum class BuildingSizeTypes
{
    UNKNOWN = 0,
    SMALL_SIZE = 1,
    MEDIUM_SIZE = 2,
    LARGE_SIZE = 3,
    HUGE_SIZE = 4,
    GATE_SIZE = 5
};

enum class UIElementTypes
{
    UNKNOWN = 0,
    RESOURCE_PANEL = 1,
    CONTROL_PANEL = 2,
    UNIT_ICON = 3,
    NATURAL_RESOURCE_ICON = 4,
    BUILDING_ICON = 5,
    PROGRESS_BAR = 6,
    CURSOR = 7,
};

enum class TreeState
{
    DEFAULT = 0,
    STUMP = 1
};

enum ResourceType : uint8_t
{
    RT_NONE = 0,
    WOOD = 1 << 0,  // 00000001
    STONE = 1 << 1, // 00000010
    GOLD = 1 << 2,  // 00000100
    FOOD = 1 << 3,  // 00001000
};

} // namespace game

#endif