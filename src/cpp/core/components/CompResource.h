#ifndef COMPRESOURCE_H
#define COMPRESOURCE_H

#include "InGameResource.h"
#include "Property.h"
#include "Rect.h"
#include "utils/Constants.h"

#include <cstdint>

namespace core
{
class CompResource
{
  public:
    Property<InGameResource> original;

  public:
    Property<std::string> resourceName;
    Property<uint32_t> resourceAmount;

    static constexpr auto properties()
    {
        // TODO: Relying on model name as the resource name is not ideal, should be replaced
        return std::tuple{
            PropertyDesc<&CompResource::resourceName>{"Model", "name"},
            PropertyDesc<&CompResource::resourceAmount>{"NaturalResource", "resource_amount"}};
    }

  public:
    uint32_t remainingAmount = 0;

    // Relying on externally provided own position to avoid coupling CompTransform with this class
    Rect<float> getLandInFeetRect(const Feet& position) const
    {
        // Get the actual bottom corner (but not 10 feet like in snapped version)
        Tile tile = position.toTile() + 1;
        Feet corner = tile.toFeet();

        float x = corner.x - 1 * Constants::FEET_PER_TILE;
        float y = corner.y - 1 * Constants::FEET_PER_TILE;
        float w = 1 * Constants::FEET_PER_TILE;
        float h = 1 * Constants::FEET_PER_TILE;
        return Rect<float>(x, y, w, h);
    }

};
} // namespace core

#endif