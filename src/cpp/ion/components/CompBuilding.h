#ifndef COMPBUILDING_H
#define COMPBUILDING_H

#include "Feet.h"
#include "Rect.h"
#include "utils/Constants.h"
#include "utils/Size.h"

namespace ion
{
class CompBuilding
{
  public:
    bool isPlacing = false;
    bool validPlacement = true;
    Size size{0, 0};
    uint32_t lineOfSight = 0; // LOS in feet
    uint8_t dropOffForResourceType = Constants::RESOURCE_TYPE_NONE;

    // Relying on externally provided own position to avoid coupling CompTransform with this class
    Rect<float> getLandInFeetRect(const Feet& position) const
    {
        // Get the actual bottom corner (but not 10 feet like in snapped version)
        Tile tile = position.toTile() + 1;
        Feet corner = tile.toFeet();

        float x = corner.x - size.width * Constants::FEET_PER_TILE;
        float y = corner.y - size.height * Constants::FEET_PER_TILE;
        float w = size.width * Constants::FEET_PER_TILE;
        float h = size.height * Constants::FEET_PER_TILE;
        return Rect<float>(x, y, w, h);
    }

    // Relying on externally provided own position to avoid coupling CompTransform with this class
    Feet getTileSnappedPosition(const Feet& position) const
    {
        // Building bottom corner (in isometric view, but not logical view) is almost at the
        // bottom corner of the tile (i.e. 10 feet before next tile)
        Tile tile = position.toTile() + Tile(1, 1);
        return tile.toFeet() - 10;
    }
};

} // namespace ion

#endif