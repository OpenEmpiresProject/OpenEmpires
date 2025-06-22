#include "FogOfWar.h"

#include "GameState.h"
#include "ServiceRegistry.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"

using namespace ion;

FogOfWar::FogOfWar(/* args */)
{
    m_coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
}

FogOfWar::~FogOfWar()
{
}

void FogOfWar::init(uint32_t width, uint32_t height, RevealStatus initialFill)
{
    m_width = width;
    m_height = height;

    m_map.resize(width);

    for (uint32_t i = 0; i < width; ++i)
    {
        m_map[i].resize(height);
        for (uint32_t j = 0; j < height; ++j)
        {
            m_map[i][j] = initialFill;
        }
    }
}

void FogOfWar::markAsExplored(uint32_t x, uint32_t y)
{
    if (x >= 0 && x < m_width && y >= 0 && y < m_height)
    {
        m_map[x][y] = RevealStatus::EXPLORED;
    }
}

void FogOfWar::markAsExplored(const Feet& feetPos)
{
    auto tilePos = feetPos.toTile();
    markAsExplored(tilePos.x, tilePos.y);
}

void FogOfWar::markAsExplored(const Feet& feetPos, uint8_t lineOfSight)
{
    auto tilePos = feetPos.toTile();
    markRadius(tilePos.x, tilePos.y, lineOfSight, RevealStatus::EXPLORED);
}

void FogOfWar::markAsVisible(uint32_t tileX, uint32_t tileY, uint8_t lineOfSight)
{
    markRadius(tileX, tileY, lineOfSight, RevealStatus::VISIBLE);
}

void FogOfWar::markAsVisible(const Feet& feetPos, uint8_t lineOfSight)
{
    auto tilePos = feetPos.toTile();
    markAsVisible(tilePos.x, tilePos.y, lineOfSight);
}

void FogOfWar::markRadius(uint32_t tileX, uint32_t tileY, uint8_t lineOfSight, RevealStatus type)
{
    const int32_t minX = std::max<int32_t>(0, static_cast<int32_t>(tileX) - lineOfSight);
    const int32_t maxX = std::min<uint32_t>(m_width - 1, tileX + lineOfSight);
    const int32_t minY = std::max<int32_t>(0, static_cast<int32_t>(tileY) - lineOfSight);
    const int32_t maxY = std::min<uint32_t>(m_height - 1, tileY + lineOfSight);

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

const std::vector<std::vector<RevealStatus>>& FogOfWar::getMap() const
{
    return m_map;
}

RevealStatus FogOfWar::getRevealMode(uint32_t tileX, uint32_t tileY) const
{
    return m_map[tileX][tileY];
}

RevealStatus FogOfWar::getRevealMode(const Tile& tilePos) const
{
    return m_map[tilePos.x][tilePos.y];
}

void FogOfWar::setRevealMode(uint32_t tileX, uint32_t tileY, RevealStatus type)
{
    if (type == m_map[tileX][tileY])
        return;

    if (type == RevealStatus::EXPLORED || type == RevealStatus::VISIBLE)
    {
        auto& gameState = GameState::getInstance();
        auto fowTileEntity =
            gameState.gameMap.getEntity(MapLayerType::FOG_OF_WAR, {(int) tileX, (int) tileY});
        if (fowTileEntity != entt::null)
        {
            auto [info, dirty] = gameState.getComponents<CompEntityInfo, CompDirty>(fowTileEntity);

            info.isDestroyed = true;
            dirty.markDirty(fowTileEntity);
            gameState.gameMap.removeAllEntities(MapLayerType::FOG_OF_WAR,
                                                {(int) tileX, (int) tileY});
        }
    }
    m_map[tileX][tileY] = type;
}

void FogOfWar::disable()
{
    auto& gameState = GameState::getInstance();

    for (uint32_t i = 0; i < m_width; ++i)
    {
        for (uint32_t j = 0; j < m_height; ++j)
        {
            auto fowTileEntity =
                gameState.gameMap.getEntity(MapLayerType::FOG_OF_WAR, {(int) i, (int) j});
            if (fowTileEntity != entt::null)
            {
                auto [info, dirty] =
                    gameState.getComponents<CompEntityInfo, CompDirty>(fowTileEntity);

                info.isDestroyed = true;
                dirty.markDirty(fowTileEntity);
            }
        }
    }
}

void FogOfWar::enable()
{
    auto& gameState = GameState::getInstance();

    for (uint32_t i = 0; i < m_width; ++i)
    {
        for (uint32_t j = 0; j < m_height; ++j)
        {
            auto fowTileEntity =
                gameState.gameMap.getEntity(MapLayerType::FOG_OF_WAR, {(int) i, (int) j});
            if (fowTileEntity != entt::null)
            {
                auto [info, dirty] =
                    gameState.getComponents<CompEntityInfo, CompDirty>(fowTileEntity);

                info.isDestroyed = false;
                dirty.markDirty(fowTileEntity);
            }
        }
    }
}
