#ifndef TYPES_H
#define TYPES_H

#include "Constants.h"

#include <bitset>

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

} // namespace utils

#endif