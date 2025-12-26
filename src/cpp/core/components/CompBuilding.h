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
    // Indicate what are the resource types this building accepts to drop.
    // It may support more than 1 resource type, the following act as a flag.
    Property<uint8_t> dropOffForResourceType;
    Property<bool> connectedConstructionsAllowed;
    Property<BuildingOrientation> defaultOrientation = {BuildingOrientation::NO_ORIENTATION};
    Property<std::map<int, int>> visualVariationByProgress;

  public:
    Property<std::vector<std::string>> acceptedResourceNames;
    // Property<std::string> constructionSiteName;

  public:
    bool validPlacement = true;
    uint32_t constructionProgress = 0; // out of 100
    // Lower bound represents the entity variation to be used based on the progress of the
    // construction.
    bool isInStaticMap = false;
    BuildingOrientation orientation = BuildingOrientation::NO_ORIENTATION;
    LandArea landArea;

    // Check if the building supports drop-off for a specific resource type
    bool acceptResource(uint8_t resourceType) const;
    std::vector<uint8_t> getAcceptedResources() const;
    bool isConstructed() const;
    bool isConstructing() const;
    int getVariationByConstructionProgress() const;
    Rect<float> getLandInFeetRect() const;
    static Rect<float> getLandInFeetRect(const LandArea& area);
    Feet getSnappedBuildingCenter(const Feet& position) const;
    void updateLandArea(const Feet& center);
};

} // namespace core

#endif