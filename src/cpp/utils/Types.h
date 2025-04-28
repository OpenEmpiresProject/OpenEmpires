#ifndef TYPES_H
#define TYPES_H

#include "Constants.h"

#include <bitset>
#include <string>

namespace utils
{
using Signature = std::bitset<utils::Constants::MAX_COMPONENTS>;
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
    NORTHWEST
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

} // namespace utils

#endif