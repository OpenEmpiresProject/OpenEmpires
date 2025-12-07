#include "FogOfWar.h"

#include "Feet.h"
#include "StateManager.h"
#include "Tile.h"
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
    markAsExplored(feetPos.toTile());
}

void FogOfWar::markAsExplored(const Feet& feetPos, uint32_t lineOfSight)
{
    markRadius(feetPos.toTile(), lineOfSight / Constants::FEET_PER_TILE, RevealStatus::EXPLORED);
}

void FogOfWar::markAsExplored(const LandArea& landArea, uint32_t lineOfSight)
{
    markRadius(landArea, lineOfSight / Constants::FEET_PER_TILE, RevealStatus::EXPLORED);
}

void FogOfWar::markAsVisible(const Tile& tilePos, uint32_t lineOfSight)
{
    markRadius(tilePos, lineOfSight / Constants::FEET_PER_TILE, RevealStatus::VISIBLE);
}

void FogOfWar::markAsVisible(const Feet& feetPos, uint32_t lineOfSight)
{
    auto tilePos = feetPos.toTile();
    markAsVisible(tilePos, lineOfSight);
}

void FogOfWar::markRadius(const Tile& tile, uint8_t lineOfSight, RevealStatus type)
{
    markRadius(tile, lineOfSight, m_map.dimensions(),
               [&](const Tile& t) { setRevealStatus(t, type); });
}

void FogOfWar::markRadius(const LandArea& landArea, uint8_t lineOfSight, RevealStatus type)
{
    markRadius(landArea, lineOfSight, m_map.dimensions(),
               [&](const Tile& t) { setRevealStatus(t, type); });
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