#include "CompBuilding.h"

using namespace core;

int CompBuilding::getVariationByConstructionProgress() const
{
    auto props = CompBuilding::properties();
    debug_assert(visualVariationByProgress.size() > 0, "Building's visual variation map is empty");

    auto it = visualVariationByProgress.lower_bound(constructionProgress);
    return it->second;
}

// Compute bounding box from landArea tiles using tile centers to ensure full-tile coverage.
Rect<float> CompBuilding::getLandInFeetRect() const
{
    return getLandInFeetRect(landArea);
}

core::Rect<float> CompBuilding::getLandInFeetRect(const LandArea& area)
{
    debug_assert(not area.tiles.empty(), "Land area is empty for building");

    float minCx = std::numeric_limits<float>::infinity();
    float maxCx = -std::numeric_limits<float>::infinity();
    float minCy = std::numeric_limits<float>::infinity();
    float maxCy = -std::numeric_limits<float>::infinity();

    // Calculate bounding centers
    for (const auto& t : area.tiles)
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
    int nearestX = 0;
    int nearestY = 0;

    if (orientation == BuildingOrientation::DIAGONAL_FORWARD)
    {
        // Building sizes are independent of orientations and doesn't cause any issues
        // if the building is a square. But when it is not, the largest value
        // (between width and height) comes first. i.e. sizes are defined by assuming
        // those are left aligned. eg: a gate would be 4x1
        //
        width = size.value().height;
        height = size.value().width;
    }
    else if (orientation == BuildingOrientation::HORIZONTAL or
             orientation == BuildingOrientation::VERTICAL)
    {
        // For horizontal oriented building larger than a single tile should be centered to tile
        // edges. But single tile horizontal buildings should be centered to tile center.
        // This will take care first, and generic path of the rest of the function handle second.
        //
        if (width > 1 or height > 1)
        {
            nearestX = roundToNearestTileEdge(position.x);
            nearestY = roundToNearestTileEdge(position.y);

            return Feet(nearestX, nearestY);
        }
    }

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
    landArea.tiles.clear();

    int width = size.value().width;
    int height = size.value().height;

    if (orientation == BuildingOrientation::DIAGONAL_FORWARD)
    {
        width = size.value().height;
        height = size.value().width;
    }
    else if (orientation == BuildingOrientation::HORIZONTAL)
    {
        debug_assert(width == 1 or height == 1, "Either side should be only 1 tile size. Any other "
                                                "non-square building size is not supported");
        const float largestSideLength = std::max(width, height);
        const float halfLargest = largestSideLength / 2;
        const int diff = halfLargest * Constants::FEET_PER_TILE - 10;

        const Feet leftCorner = center + Feet(-diff, diff);
        const Tile leftCornerzTile = leftCorner.toTile();

        for (int i = 0; i < largestSideLength; ++i)
        {
            landArea.tiles.emplace_back(leftCornerzTile + Tile(i, -i));
        }
        return;
    }
    else if (orientation == BuildingOrientation::VERTICAL)
    {
        debug_assert(width == 1 or height == 1, "Either side should be only 1 tile size. Any other "
                                                "non-square building size is not supported");
        const float largestSideLength = std::max(width, height);
        const float halfLargest = largestSideLength / 2;
        const int diff = halfLargest * Constants::FEET_PER_TILE - 10;

        const Feet topCorner = center + Feet(-diff, -diff);
        const Tile topCornerzTile = topCorner.toTile();

        for (int i = 0; i < largestSideLength; ++i)
        {
            landArea.tiles.emplace_back(topCornerzTile + Tile(i, i));
        }
        return;
    }

    float halfSizeWidth = (float) width / 2;
    float halfSizeHeight = (float) height / 2;

    int dx = halfSizeWidth * Constants::FEET_PER_TILE - 10;
    int dy = halfSizeHeight * Constants::FEET_PER_TILE - 10;

    Feet bottomCorner = center + Feet(dx, dy);
    Tile bottomCornerTile = bottomCorner.toTile();

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

std::vector<uint8_t> CompBuilding::getAcceptedResources() const
{
    std::vector<uint8_t> resources;
    for (uint8_t i = 1; i < 8; ++i)
    {
        if (acceptResource(i))
        {
            resources.push_back(i);
        }
    }
    return resources;
}
