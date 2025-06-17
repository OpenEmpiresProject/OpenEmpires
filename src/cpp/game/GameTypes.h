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
    ET_UI_ELEMENT = 10000
};

enum EntitySubTypes
{
    UI_WINDOW = 0,
    UI_BUTTON = 1,
    UI_LABEL = 2
};

enum ResourceType : uint8_t
{
    WOOD = 0,
    STONE = 1,
    GOLD = 2
};

} // namespace game

#endif