#ifndef TYPES_H
#define TYPES_H

#include "utils/Constants.h"

#include <bitset>
#include <memory>
#include <string>
#include <utility>

namespace ion
{
template <typename T> using Ref = std::shared_ptr<T>;

template <typename T, typename... Args> Ref<T> CreateRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

using Signature = std::bitset<Constants::MAX_COMPONENTS>;

enum class WorldSizeType
{
    DEMO,
    TINY,
    MEDIUM,
    GIANT
};

enum class Direction
{
    NORTH = 0,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST,
    NONE
};

static std::string directionToString(Direction direction)
{
    switch (direction)
    {
    case Direction::NORTH:
        return "NORTH";
    case Direction::NORTHEAST:
        return "NORTHEAST";
    case Direction::EAST:
        return "EAST";
    case Direction::SOUTHEAST:
        return "SOUTHEAST";
    case Direction::SOUTH:
        return "SOUTH";
    case Direction::SOUTHWEST:
        return "SOUTHWEST";
    case Direction::WEST:
        return "WEST";
    case Direction::NORTHWEST:
        return "NORTHWEST";
    default:
        return "UNKNOWN";
    }
}

enum Actions
{
    IDLE = 0,
    MOVE = 1,
    CHOPPING = 2,
    MINING = 3,
    BUILDING = 4
};

enum class RevealStatus
{
    NONE = 0,
    UNEXPLORED,
    EXPLORED,
    VISIBLE
};

template <typename T> inline int toInt(const T& t)
{
    return static_cast<int>(t);
}

} // namespace ion

#endif