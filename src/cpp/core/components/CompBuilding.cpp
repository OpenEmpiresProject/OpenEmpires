#include "CompBuilding.h"

using namespace core;

int CompBuilding::getVariationByConstructionProgress() const
{
    debug_assert(visualVariationByProgress.size() > 0, "Building's visual variation map is empty");

    auto it = visualVariationByProgress.lower_bound(constructionProgress);
    return it->second;
}

// Compute bounding box from landArea tiles using tile centers to ensure full-tile coverage.
Rect<float> CompBuilding::getLandInFeetRect() const
{
    debug_assert(not landArea.tiles.empty(), "Land area is empty for building");

    float minCx = std::numeric_limits<float>::infinity();
    float maxCx = -std::numeric_limits<float>::infinity();
    float minCy = std::numeric_limits<float>::infinity();
    float maxCy = -std::numeric_limits<float>::infinity();

    // Calculate bounding centers
    for (const auto& t : landArea.tiles)
    {
        Feet c = t.centerInFeet();
        minCx = std::min(minCx, c.x);
        maxCx = std::max(maxCx, c.x);
        minCy = std::min(minCy, c.y);
        maxCy = std::max(maxCy, c.y);
    }

    const float halfTile = static_cast<float>(Constants::FEET_PER_TILE) / 2.0f;

    float left = minCx - halfTile;
    float top = minCy - halfTile;
    float width = (maxCx - minCx) + static_cast<float>(Constants::FEET_PER_TILE);
    float height = (maxCy - minCy) + static_cast<float>(Constants::FEET_PER_TILE);

    return Rect<float>(left, top, width, height);
}

int roundToNearestTileCenter(float x)
{
    const int feetPerHalfTile = Constants::FEET_PER_TILE / 2;

    // Step 1: nearest multiple of 128
    int n = std::round(x / feetPerHalfTile);

    // Step 2: convert to nearest ODD n
    if (n % 2 == 0)
    {
        // Two candidates: n-1 and n+1 (both odd)
        int lower = (n - 1) * feetPerHalfTile;
        int upper = (n + 1) * feetPerHalfTile;
        return (std::abs(x - lower) < std::abs(x - upper)) ? lower : upper;
    }

    // n is already odd
    return n * feetPerHalfTile;
}

int roundToNearestTileEdge(float x)
{
    return Constants::FEET_PER_TILE * std::round(x / Constants::FEET_PER_TILE);
}

Feet CompBuilding::getSnappedBuildingCenter(const Feet& position) const
{
    int width = size.value().width;
    int height = size.value().height;

    if (orientation == BuildingOrientation::RIGHT_ANGLED)
    {
        width = size.value().height;
        height = size.value().width;
    }

    int nearestX = 0;
    int nearestY = 0;

    // center will be snapped to tile border if the the size is even, else to tile center
    //
    if (width % 2 == 0)
    {
        nearestX = roundToNearestTileEdge(position.x);
    }
    else
    {
        nearestX = roundToNearestTileCenter(position.x);
    }

    if (height % 2 == 0)
    {
        nearestY = roundToNearestTileEdge(position.y);
    }
    else
    {
        nearestY = roundToNearestTileCenter(position.y);
    }

    return Feet(nearestX, nearestY);
}

void CompBuilding::updateLandArea(const Feet& center)
{
    int width = size.value().width;
    int height = size.value().height;

    if (orientation == BuildingOrientation::RIGHT_ANGLED)
    {
        width = size.value().height;
        height = size.value().width;
    }

    float halfSizeWidth = (float) width / 2;
    float halfSizeHeight = (float) height / 2;

    int dx = halfSizeWidth * Constants::FEET_PER_TILE - 10;
    int dy = halfSizeHeight * Constants::FEET_PER_TILE - 10;

    Feet bottomCorner = center + Feet(dx, dy);
    Tile bottomCornerTile = bottomCorner.toTile();

    landArea.tiles.clear();

    // Not a square building, needs specially handling to find the anchor depending on the
    // orientation
    /*if (size.value().width != size.value().height)
    {
        if (orientation == BuildingOrientation::RIGHT_ANGLED)
        {
            bottomCornerTile.y += size.value().width / 2;
        }
    }*/

    for (int x = 0; x < width; ++x)
    {
        for (int y = 0; y < height; ++y)
        {
            landArea.tiles.emplace_back(bottomCornerTile - Tile(x, y));
        }
    }
}

bool CompBuilding::isConstructed() const
{
    return constructionProgress >= 100;
}

bool CompBuilding::acceptResource(uint8_t resourceType) const
{
    return dropOffForResourceType & resourceType;
}
