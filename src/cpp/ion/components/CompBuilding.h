#ifndef COMPBUILDING_H
#define COMPBUILDING_H

#include "Feet.h"
#include "Rect.h"
#include "debug.h"
#include "utils/Constants.h"
#include "utils/Size.h"

#include <map>

namespace ion
{
class CompBuilding
{
  public:
    bool validPlacement = true;
    Size size{0, 0};
    uint32_t lineOfSight = 0;          // LOS in feet
    uint32_t constructionProgress = 0; // out of 100
    // Lower bound represents the entity variation to be used based on the progress of the
    // construction.
    std::map<uint32_t, uint32_t> visualVariationByProgress = {{0, 0}};
    bool isInStaticMap = false;

    uint32_t getVisualVariation() const
    {
        debug_assert(visualVariationByProgress.size() > 0,
                     "Building's visual variation map is empty");

        auto it = visualVariationByProgress.lower_bound(constructionProgress);
        return it->second;
    }

    // Add a resource type to the drop-off list
    inline void addDropOff(uint8_t type)
    {
        dropOffForResourceType |= type;
    }

    // Check if the building supports drop-off for a specific resource type
    inline bool canDropOff(uint8_t type) const
    {
        return dropOffForResourceType & type;
    }

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

  private:
    // Indicate what are the resource types this building accepts to drop.
    // It may support more than 1 resource type, the following act as a flag.
    uint8_t dropOffForResourceType = Constants::RESOURCE_TYPE_NONE;
};

} // namespace ion

#endif