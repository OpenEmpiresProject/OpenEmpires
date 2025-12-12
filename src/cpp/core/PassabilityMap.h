#ifndef CORE_PASSABILITYMAP_H
#define CORE_PASSABILITYMAP_H

#include "Flat2DArray.h"
#include "Tile.h"

#include <optional>

namespace core
{
enum class TerrainPassability
{
    PASSABLE_FOR_LAND_UNITS,
    PASSABLE_FOR_WATER_UNITS,
    PASSABLE_FOR_ANY, // Lagoon
    BLOCKED_FOR_ANY   // Indestructible natural/artificial resource
};

enum class DynamicPassability
{
    PASSABLE_FOR_ANY,            // No constructions
    BLOCKED_FOR_ANY,             // Any construction other than gate, mine, tree
    PASSABLE_FOR_OWNER_OR_ALLIED // There should be a owner associated
};

class PassabilityMap
{
  public:
    void init(uint32_t width, uint32_t height);
    void setTileTerrainPassability(const Tile& tile, TerrainPassability passability);
    void setTileDynamicPassability(const Tile& tile, DynamicPassability passability);
    void setTileDynamicPassability(const Tile& tile, DynamicPassability passability, uint8_t owner);
    bool isPassableFor(const Tile& tile, uint8_t playerId) const;

    Size getSize() const;

  private:
    struct PassabilityOwnership
    {
        DynamicPassability passability = DynamicPassability::PASSABLE_FOR_ANY;
        std::optional<uint8_t> owner = std::nullopt; // Player id

        PassabilityOwnership(DynamicPassability passability, uint8_t owner)
            : passability(passability), owner(owner)
        {
        }

        PassabilityOwnership(DynamicPassability passability) : passability(passability)
        {
        }
    };
    Flat2DArray<PassabilityOwnership> m_dynamicPassability;
    Flat2DArray<TerrainPassability> m_terrainPassability;
};
} // namespace core

#endif // CORE_PASSABILITYMAP_H
