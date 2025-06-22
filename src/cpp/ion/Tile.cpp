#include "Tile.h"

#include "Coordinates.h"
#include "Feet.h"
#include "ServiceRegistry.h"

#include <limits>

using namespace ion;

const ion::Tile ion::Tile::null =
    ion::Tile(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());

Feet Tile::toFeet() const
{
    return Coordinates::tilesToFeet(*this);
}

Feet Tile::centerInFeet() const
{
    return Coordinates::getTileCenterInFeet(*this);
}
