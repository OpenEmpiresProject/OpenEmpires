#include "PassabilityMap.h"

#include "debug.h"

using namespace core;

void PassabilityMap::init(uint32_t width, uint32_t height)
{
    m_terrainPassability.resize(width, height, TerrainPassability::PASSABLE_FOR_ANY);
    m_dynamicPassability.resize(width, height,
                                DynamicPassability(DynamicPassability::PASSABLE_FOR_ANY));
}

void PassabilityMap::setTileTerrainPassability(const Tile& tile, TerrainPassability passability)
{
    m_terrainPassability.at(tile.x, tile.y) = passability;
}

void PassabilityMap::setTileDynamicPassability(const Tile& tile, DynamicPassability passability)
{
    debug_assert(passability != DynamicPassability::PASSABLE_FOR_OWNER_OR_ALLIED,
                 "Cannot mark passable for owner without specifying the owner");

    m_dynamicPassability.at(tile.x, tile.y) = DynamicPassability(passability);
}

void PassabilityMap::setTileDynamicPassability(const Tile& tile,
                                               DynamicPassability passability,
                                               uint8_t owner)
{
    m_dynamicPassability.at(tile.x, tile.y) = PassabilityOwnership(passability, owner);
}

bool PassabilityMap::isPassableFor(const Tile& tile, uint8_t playerId) const
{
    // TODO - check for land, water type
    const auto& currentPassability = m_dynamicPassability.at(tile.x, tile.y);

    return m_terrainPassability.at(tile.x, tile.y) == TerrainPassability::PASSABLE_FOR_ANY and
           (currentPassability.passability == DynamicPassability::PASSABLE_FOR_ANY or
            (currentPassability.passability == DynamicPassability::PASSABLE_FOR_OWNER_OR_ALLIED and
             currentPassability.owner == playerId));
}

Size PassabilityMap::getSize() const
{
    return m_terrainPassability.dimensions();
}
