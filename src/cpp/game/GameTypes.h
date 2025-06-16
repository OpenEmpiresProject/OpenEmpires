#ifndef GAMETYPES_H
#define GAMETYPES_H

namespace game
{
enum BaseEntityTypes
{
    UI_ELEMENT = 10000
};

enum BaseEntitySubTypes
{
    UI_WINDOW = 0,
    UI_BUTTON = 1,
    UI_LABEL = 2
};

enum ResourceType : uint8_t
{
    WOOD = 0
};

} // namespace game

#endif