#ifndef TYPES_H
#define TYPES_H

#include "Vec2Base.h"
#include "utils/Constants.h"

#include <bitset>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

namespace core
{
template <typename T> using Ref = std::shared_ptr<T>;

template <typename T, typename... Args> Ref<T> CreateRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

using Signature = std::bitset<Constants::MAX_COMPONENTS>;

struct GenericTag
{
};
using Vec2 = Vec2Base<float, GenericTag>;
using ImageId = int64_t;

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

enum UnitAction
{
    IDLE = 0,
    MOVE = 1,
    CHOPPING = 2,
    MINING = 3,
    BUILDING = 4,
    CARRYING_LUMBER = 5,
    CARRYING_GOLD = 6,
    CARRYING_STONE = 7,
};

enum class RevealStatus : uint8_t
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

} // namespace core

#endif