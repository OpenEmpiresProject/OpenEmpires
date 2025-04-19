#ifndef TYPES_H
#define TYPES_H

#include <bitset>
#include "Constants.h"

namespace utils
{
    using Signature = std::bitset<utils::Constants::MAX_COMPONENTS>;
    enum class WorldSizeType
    {
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
    
}


#endif