#ifndef GAMETYPES_H
#define GAMETYPES_H

namespace game
{
enum EntityTypes
{
    ET_UNKNOWN = 0,

    ET_TILE = 2,
    ET_VILLAGER = 3,
    ET_TREE = 4,
    ET_MILL = 5,
    ET_MARKETPLACE = 6,
    ET_STONE = 7,
    ET_GOLD = 8,
    ET_LUMBER_CAMP = 9,
    ET_MINING_CAMP = 10,
    ET_CONSTRUCTION_SITE = 11,
    ET_TOWN_CENTER = 12,

    ET_UI_ELEMENT = 10000
};

enum EntitySubTypes
{
    EST_UI_RESOURCE_PANEL = 1,
    EST_UI_CONTROL_PANEL = 2,
    UI_UNIT_ICON = 3,
    UI_NATURAL_RESOURCE_ICON = 4,
    UI_BUILDING_ICON = 5,
    UI_PROGRESS_BAR = 6,

    EST_SMALL_SIZE = 1,
    EST_MEDIUM_SIZE = 2,
    EST_LARGE_SIZE = 3,

    EST_DEFAULT = 0,
    EST_CHOPPED_TREE = 1,
    EST_TREE_SHADOW = 2
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