#ifndef FOGOFWAR_H
#define FOGOFWAR_H

#include "Flat2DArray.h"
#include "components/CompBuilding.h"
#include "utils/Types.h"

#include <cstdint>
#include <vector>

namespace core
{
class Feet;
class Tile;
class Size;

class FogOfWar
{
  public:
    void init(uint32_t width, uint32_t height, RevealStatus initialFill);
    void markAsExplored(const Tile& pos);
    void markAsExplored(const Feet& pos);
    void markAsExplored(const Feet& pos, uint32_t lineOfSight);
    void markAsExplored(const LandArea& landArea, uint32_t lineOfSight);
    void markAsVisible(const Tile& pos, uint32_t lineOfSight);
    void markAsVisible(const Feet& pos, uint32_t lineOfSight);

    void setRevealStatus(const Tile& tilePos, RevealStatus type);
    RevealStatus getRevealStatus(const Tile& tilePos) const;
    bool isExplored(const Tile& tile) const;

    template <typename Callback>
    static void markRadius(const Tile& tile,
                           uint8_t lineOfSightInTiles,
                           const Size& tileMapSize,
                           Callback cb)
    {
        const int32_t minX =
            std::max<int32_t>(0, static_cast<int32_t>(tile.x) - lineOfSightInTiles);
        const int32_t maxX = std::min<uint32_t>(tileMapSize.width - 1, tile.x + lineOfSightInTiles);
        const int32_t minY =
            std::max<int32_t>(0, static_cast<int32_t>(tile.y) - lineOfSightInTiles);
        const int32_t maxY =
            std::min<uint32_t>(tileMapSize.height - 1, tile.y + lineOfSightInTiles);

        const uint32_t radiusSq = lineOfSightInTiles * lineOfSightInTiles;

        for (int32_t x = minX; x <= maxX; ++x)
        {
            for (int32_t y = minY; y <= maxY; ++y)
            {
                uint32_t dx = x - tile.x;
                uint32_t dy = y - tile.y;
                if (dx * dx + dy * dy <= radiusSq)
                {
                    cb(Tile(x, y));
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
    //    Break out of the inner loop early when a distance <= radiusSq is found to avoid extra
    //    work.
    // 6. Use safe integer types (int64_t for squared computations) to avoid overflow for larger
    // maps.
    template <typename Callback>
    static void markRadius(const LandArea& landArea,
                           uint8_t lineOfSightInTiles,
                           const Size& tileMapSize,
                           Callback cb)
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

        // Expand by lineOfSightInTiles and clamp to map bounds
        int32_t minX = std::max<int32_t>(0, areaMinX - static_cast<int32_t>(lineOfSightInTiles));
        int32_t maxX = std::min<int32_t>(static_cast<int32_t>(tileMapSize.width) - 1,
                                         areaMaxX + static_cast<int32_t>(lineOfSightInTiles));
        int32_t minY = std::max<int32_t>(0, areaMinY - static_cast<int32_t>(lineOfSightInTiles));
        int32_t maxY = std::min<int32_t>(static_cast<int32_t>(tileMapSize.height) - 1,
                                         areaMaxY + static_cast<int32_t>(lineOfSightInTiles));

        const int64_t radiusSq =
            static_cast<int64_t>(lineOfSightInTiles) * static_cast<int64_t>(lineOfSightInTiles);

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
                    cb(Tile(x, y));
                }
            }
        }
    }

  private:
    void markRadius(const Tile& tile, uint8_t lineOfSight, RevealStatus type);
    void markRadius(const LandArea& landArea, uint8_t lineOfSight, RevealStatus type);

  private:
    Flat2DArray<RevealStatus> m_map;
};
} // namespace core

#endif