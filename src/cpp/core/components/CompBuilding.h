#ifndef COMPBUILDING_H
#define COMPBUILDING_H

#include "Feet.h"
#include "Property.h"
#include "Rect.h"
#include "Tile.h"
#include "debug.h"
#include "utils/Constants.h"
#include "utils/Size.h"

#include <map>
#include <unordered_map>

namespace core
{
class CompBuilding
{
  public:
    Property<Size> size;
    Property<uint32_t> lineOfSight; // In Feet
    // Indicate what are the resource types this building accepts to drop.
    // It may support more than 1 resource type, the following act as a flag.
    Property<uint8_t> dropOffForResourceType;
    Property<std::unordered_map<BuildingOrientation, uint32_t>> framesByOrientation;
    Property<bool> connectedConstructionsAllowed;

  public:
    bool validPlacement = true;
    uint32_t constructionProgress = 0; // out of 100
    // Lower bound represents the entity variation to be used based on the progress of the
    // construction.
    std::map<int, int> visualVariationByProgress = {{0, 0}};
    uint32_t constructionSiteEntitySubType = 0;
    bool isInStaticMap = false;
    BuildingOrientation orientation = BuildingOrientation::DEFAULT;

    bool isConstructed() const
    {
        return constructionProgress >= 100;
    }

    int getVariationByConstructionProgress() const
    {
        debug_assert(visualVariationByProgress.size() > 0,
                     "Building's visual variation map is empty");

        auto it = visualVariationByProgress.lower_bound(constructionProgress);
        return it->second;
    }

    // Check if the building supports drop-off for a specific resource type
    inline bool acceptResource(uint8_t resourceType) const
    {
        return dropOffForResourceType & resourceType;
    }

    // Relying on externally provided own position to avoid coupling CompTransform with this class
    Rect<float> getLandInFeetRect(const Feet& position) const
    {
        // Get the actual bottom corner (but not 10 feet like in snapped version)
        Tile tile = position.toTile() + 1;
        Feet corner = tile.toFeet();

        const auto& sizeVal = size.value();

        float x = corner.x - sizeVal.width * Constants::FEET_PER_TILE;
        float y = corner.y - sizeVal.height * Constants::FEET_PER_TILE;
        float w = sizeVal.width * Constants::FEET_PER_TILE;
        float h = sizeVal.height * Constants::FEET_PER_TILE;
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

    Feet getBuildingCenter(const Feet& anchor) const
    {
        const float halfWidth = (float) size.value().width / 2;
        const float halfHeight = (float) size.value().height / 2;

        return anchor -
               Feet(halfWidth * Constants::FEET_PER_TILE, halfHeight * Constants::FEET_PER_TILE);
    }

    Feet getBuildingAnchor(const Feet& center) const
    {
        const float halfWidth = (float) size.value().width / 2;
        const float halfHeight = (float) size.value().height / 2;

        return center +
               Feet(halfWidth * Constants::FEET_PER_TILE, halfHeight * Constants::FEET_PER_TILE);
    }
};

} // namespace core

#endif