#include "FogOfWar.h"

#include "Feet.h"
#include "GameState.h"
#include "Tile.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "utils/Size.h"

using namespace core;

void FogOfWar::init(uint32_t width, uint32_t height, RevealStatus initialFill)
{
    m_map = Flat2DArray<RevealStatus>(width, height, initialFill);
}

void FogOfWar::markAsExplored(uint32_t x, uint32_t y)
{
    if (x >= 0 && x < m_map.width() && y >= 0 && y < m_map.height())
    {
        m_map.at(x, y) = RevealStatus::EXPLORED;
    }
}

void FogOfWar::markAsExplored(const Feet& feetPos)
{
    auto tilePos = feetPos.toTile();
    markAsExplored(tilePos.x, tilePos.y);
}

void FogOfWar::markAsExplored(const Feet& feetPos, uint32_t lineOfSight)
{
    auto tilePos = feetPos.toTile();
    markRadius(tilePos.x, tilePos.y, lineOfSight / Constants::FEET_PER_TILE,
               RevealStatus::EXPLORED);
}

void FogOfWar::markAsExplored(const Feet& feetPos, const Size& size, uint32_t lineOfSight)
{
    auto tilePos = feetPos.toTile();
    markRadius(tilePos, size, lineOfSight / Constants::FEET_PER_TILE, RevealStatus::EXPLORED);
}

void FogOfWar::markAsVisible(uint32_t tileX, uint32_t tileY, uint32_t lineOfSight)
{
    markRadius(tileX, tileY, lineOfSight / Constants::FEET_PER_TILE, RevealStatus::VISIBLE);
}

void FogOfWar::markAsVisible(const Feet& feetPos, uint32_t lineOfSight)
{
    auto tilePos = feetPos.toTile();
    markAsVisible(tilePos.x, tilePos.y, lineOfSight);
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
                setRevealMode(x, y, type);
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
                setRevealMode(x, y, type);
            }
        }
    }
}

RevealStatus FogOfWar::getRevealStatus(uint32_t tileX, uint32_t tileY) const
{
    return m_map.at(tileX, tileY);
}

RevealStatus FogOfWar::getRevealStatus(const Tile& tilePos) const
{
    return m_map.at(tilePos.x, tilePos.y);
}

void FogOfWar::setRevealMode(uint32_t tileX, uint32_t tileY, RevealStatus type)
{
    m_map.at(tileX, tileY) = type;
}

bool FogOfWar::isExplored(uint32_t tileX, uint32_t tileY) const
{
    return m_map.at(tileX, tileY) == RevealStatus::EXPLORED;
}

bool FogOfWar::isExplored(const Tile& tile) const
{
    return m_map.at(tile.x, tile.y) == RevealStatus::EXPLORED;
}