#include "Feet.h"

#include "Coordinates.h"
#include "ServiceRegistry.h"
#include "Tile.h"

#include <limits>

using namespace ion;

const ion::Feet ion::Feet::null =
    ion::Feet(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());

Tile Feet::toTile() const
{
    return Coordinates::feetToTiles(*this);
}

Vec2 Feet::toVec2() const
{
    return Vec2(x, y);
}
