#include "FogOfWar.h"

#include "Feet.h"
#include "StateManager.h"
#include "Tile.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "utils/Size.h"

using namespace core;

void FogOfWar::init(uint32_t width, uint32_t height, RevealStatus initialFill)
{
    m_map = Flat2DArray<RevealStatus>(width, height, initialFill);
}

void FogOfWar::markAsExplored(const Tile& tilePos)
{
    if (tilePos.x >= 0 && tilePos.x < m_map.width() && tilePos.y >= 0 && tilePos.y < m_map.height())
    {
        m_map.at(tilePos.x, tilePos.y) = RevealStatus::EXPLORED;
    }
}

void FogOfWar::markAsExplored(const Feet& feetPos)
{
    auto tilePos = feetPos.toTile();
    markAsExplored(tilePos);
}

void FogOfWar::markAsExplored(const Feet& feetPos, uint32_t lineOfSight)
{
    auto tilePos = feetPos.toTile();
    markRadius(tilePos.x, tilePos.y, lineOfSight / Constants::FEET_PER_TILE,
               RevealStatus::EXPLORED);
}

void FogOfWar::markAsExplored(const LandArea& landArea, uint32_t lineOfSight)
{
    markRadius(landArea, lineOfSight / Constants::FEET_PER_TILE, RevealStatus::EXPLORED);
}

void FogOfWar::markAsVisible(const Tile& tilePos, uint32_t lineOfSight)
{
    markRadius(tilePos.x, tilePos.y, lineOfSight / Constants::FEET_PER_TILE, RevealStatus::VISIBLE);
}

void FogOfWar::markAsVisible(const Feet& feetPos, uint32_t lineOfSight)
{
    auto tilePos = feetPos.toTile();
    markAsVisible(tilePos, lineOfSight);
}

void FogOfWar::markRadius(uint32_t tileX, uint32_t tileY, uint8_t lineOfSight, RevealStatus type)
{
    const int32_t minX = std::max<int32_t>(0, static_cast<int32_t>(tileX) - lineOfSight);
    const int32_t maxX = std::min<uint32_t>(m_map.width() - 1, tileX + lineOfSight);
    const int32_t minY = std::max<int32_t>(0, static_cast<int32_t>(tileY) - lineOfSight);
    const int32_t maxY = std::min<uint32_t>(m_map.height() - 1, tileY + lineOfSight);

    const uint32_t radiusSq = lineOfSight * lineOfSight;

    for (int32_t x = minX; x <= maxX; ++x)
    {
        for (int32_t y = minY; y <= maxY; ++y)
        {
            uint32_t dx = x - tileX;
            uint32_t dy = y - tileY;
            if (dx * dx + dy * dy <= radiusSq)
            {
                setRevealStatus(Tile(x, y), type);
            }
        }
    }
}

void FogOfWar::markRadius(const Tile& bottomCorner,
                          const Size& size,
                          uint8_t lineOfSight,
                          RevealStatus type)
{
    // Compute building's occupied tile range
    int32_t startX = static_cast<int32_t>(bottomCorner.x) - static_cast<int32_t>(size.width) + 1;
    int32_t startY = static_cast<int32_t>(bottomCorner.y) - static_cast<int32_t>(size.height) + 1;

    // Clamp reveal bounds
    int32_t minX = std::max(0, startX - static_cast<int32_t>(lineOfSight));
    int32_t maxX = std::min(static_cast<int32_t>(m_map.width()) - 1,
                            bottomCorner.x + static_cast<int32_t>(lineOfSight));
    int32_t minY = std::max(0, startY - static_cast<int32_t>(lineOfSight));
    int32_t maxY = std::min(static_cast<int32_t>(m_map.height()) - 1,
                            bottomCorner.y + static_cast<int32_t>(lineOfSight));

    uint32_t radiusSq = lineOfSight * lineOfSight;

    for (int32_t x = minX; x <= maxX; ++x)
    {
        for (int32_t y = minY; y <= maxY; ++y)
        {
            // Find closest point inside the building's rectangle
            int32_t nearestX = std::clamp(x, startX, static_cast<int32_t>(bottomCorner.x));
            int32_t nearestY = std::clamp(y, startY, static_cast<int32_t>(bottomCorner.y));

            int32_t dx = x - nearestX;
            int32_t dy = y - nearestY;

            if (dx * dx + dy * dy <= radiusSq)
            {
                setRevealStatus(Tile(x, y), type);
            }
        }
    }
}

// Approach:
// 1. If area.tiles is empty, return early.
// 2. Compute the bounding box of the LandArea by iterating its tiles to find minX, maxX, minY,
// maxY.
// 3. Expand that bounding box by lineOfSight, then clamp to map bounds to get iteration ranges.
// 4. Precompute radiusSq = lineOfSight * lineOfSight.
// 5. For each candidate tile (x,y) in the clamped box:
//      - Find the minimum squared distance from (x,y) to any tile coordinate in area.tiles.
//        (Distance computed as (x - tile.x)^2 + (y - tile.y)^2).
//      - If the minimum squared distance <= radiusSq, call setRevealStatus(Tile(x,y), type).
//    Break out of the inner loop early when a distance <= radiusSq is found to avoid extra work.
// 6. Use safe integer types (int64_t for squared computations) to avoid overflow for larger maps.

void FogOfWar::markRadius(const LandArea& landArea, uint8_t lineOfSight, RevealStatus type)
{
    if (landArea.tiles.empty())
    {
        return;
    }

    // Compute bounding box of the area
    int32_t areaMinX = std::numeric_limits<int32_t>::max();
    int32_t areaMaxX = std::numeric_limits<int32_t>::min();
    int32_t areaMinY = std::numeric_limits<int32_t>::max();
    int32_t areaMaxY = std::numeric_limits<int32_t>::min();

    for (const auto& t : landArea.tiles)
    {
        areaMinX = std::min(areaMinX, static_cast<int32_t>(t.x));
        areaMaxX = std::max(areaMaxX, static_cast<int32_t>(t.x));
        areaMinY = std::min(areaMinY, static_cast<int32_t>(t.y));
        areaMaxY = std::max(areaMaxY, static_cast<int32_t>(t.y));
    }

    // Expand by lineOfSight and clamp to map bounds
    int32_t minX = std::max<int32_t>(0, areaMinX - static_cast<int32_t>(lineOfSight));
    int32_t maxX = std::min<int32_t>(static_cast<int32_t>(m_map.width()) - 1,
                                     areaMaxX + static_cast<int32_t>(lineOfSight));
    int32_t minY = std::max<int32_t>(0, areaMinY - static_cast<int32_t>(lineOfSight));
    int32_t maxY = std::min<int32_t>(static_cast<int32_t>(m_map.height()) - 1,
                                     areaMaxY + static_cast<int32_t>(lineOfSight));

    const int64_t radiusSq = static_cast<int64_t>(lineOfSight) * static_cast<int64_t>(lineOfSight);

    for (int32_t x = minX; x <= maxX; ++x)
    {
        for (int32_t y = minY; y <= maxY; ++y)
        {
            int64_t minDistSq = std::numeric_limits<int64_t>::max();

            // Find nearest tile in the area (by squared Euclidean distance)
            for (const auto& t : landArea.tiles)
            {
                int32_t dx = x - static_cast<int32_t>(t.x);
                int32_t dy = y - static_cast<int32_t>(t.y);
                int64_t dSq = static_cast<int64_t>(dx) * dx + static_cast<int64_t>(dy) * dy;

                if (dSq < minDistSq)
                {
                    minDistSq = dSq;
                    if (minDistSq <= radiusSq)
                    {
                        break; // close enough; no need to search further
                    }
                }
            }

            if (minDistSq <= radiusSq)
            {
                setRevealStatus(Tile(x, y), type);
            }
        }
    }
}

RevealStatus FogOfWar::getRevealStatus(const Tile& tilePos) const
{
    return m_map.at(tilePos.x, tilePos.y);
}

void FogOfWar::setRevealStatus(const Tile& tilePos, RevealStatus type)
{
    m_map.at(tilePos.x, tilePos.y) = type;
}

bool FogOfWar::isExplored(const Tile& tile) const
{
    return m_map.at(tile.x, tile.y) == RevealStatus::EXPLORED;
}