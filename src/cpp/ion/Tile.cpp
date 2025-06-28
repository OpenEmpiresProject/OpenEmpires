#include "Tile.h"

#include "Coordinates.h"
#include "Feet.h"
#include "ServiceRegistry.h"

#include <limits>

using namespace ion;


Feet Tile::toFeet() const
{
    return Coordinates::tilesToFeet(*this);
}

Feet Tile::centerInFeet() const
{
    return Coordinates::getTileCenterInFeet(*this);
}
