#include "GameState.h"

#include "Coordinates.h"
#include "PathFinderAStar.h"
#include "PathFinderBase.h"
#include "ServiceRegistry.h"
#include "components/CompBuilding.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "utils/Logger.h"

using namespace core;

GameState::GameState()
{
    m_pathFinder = CreateRef<PathFinderAStar>();
}

uint32_t GameState::createEntity()
{
    return m_registry.create();
}

void GameState::destroyEntity(uint32_t entity)
{
    m_entitiesToDestroy.push_back(entity);
}

void core::GameState::destroyAllPendingEntities()
{
    for (auto entity : m_entitiesToDestroy)
    {
        m_registry.destroy(entity);
    }
    m_entitiesToDestroy.clear();
}

bool GameState::isEntityValid(uint32_t entity) const
{
    return m_registry.valid(entity) &&
           std::find(m_entitiesToDestroy.begin(), m_entitiesToDestroy.end(), entity) ==
               m_entitiesToDestroy.end();
}

void GameState::clearAll()
{
    m_registry.clear();
}

GameState::TileMapQueryResult GameState::whatIsAt(const Vec2& screenPos)
{
    auto coordinates = ServiceRegistry::getInstance().getService<Coordinates>();

    auto clickedCellPos = coordinates->screenUnitsToTiles(screenPos);

    spdlog::debug("Clicking at grid pos {} to select", clickedCellPos.toString());

    Tile gridStartPos = clickedCellPos + Constants::MAX_SELECTION_LOOKUP_HEIGHT;
    gridStartPos.limitTo(m_gameMap.width - 1, m_gameMap.height - 1);

    Tile pos;
    TileMapQueryResult result;

    for (pos.y = gridStartPos.y; pos.y >= clickedCellPos.y; pos.y--)
    {
        for (pos.x = gridStartPos.x; pos.x >= clickedCellPos.x; pos.x--)
        {
            if (m_gameMap.isOccupied(MapLayerType::STATIC, pos))
            {
                auto entity = m_gameMap.getEntity(MapLayerType::STATIC, pos);
                if (entity != entt::null)
                {
                    if (hasComponent<CompSelectible>(entity)) [[likely]]
                    {
                        auto [select, transform] =
                            getComponents<CompSelectible, CompTransform>(entity);
                        auto entityScreenPos = coordinates->feetToScreenUnits(transform.position);

                        const auto& boundingBox = select.getBoundingBox(transform.getDirection());
                        auto screenRect = Rect<int>(entityScreenPos.x - boundingBox.x,
                                                    entityScreenPos.y - boundingBox.y,
                                                    boundingBox.w, boundingBox.h);

                        if (screenRect.contains(screenPos))
                        {
                            return {.entity = entity, .layer = MapLayerType::STATIC};
                        }
                    }
                    else if (hasComponent<CompBuilding>(entity))
                    {
                        spdlog::debug("A building at the clicked position");
                        return {.entity = entity, .layer = MapLayerType::STATIC};
                    }
                    else [[unlikely]]
                    {
                        spdlog::error("Static entity {} at {} is not selectable", entity,
                                      pos.toString());
                    }
                }
            }
            else if (m_gameMap.isOccupied(MapLayerType::ON_GROUND, pos))
            {
                auto entity = m_gameMap.getEntity(MapLayerType::ON_GROUND, pos);
                if (entity != entt::null)
                {
                    if (hasComponent<CompBuilding>(entity))
                    {
                        spdlog::debug("A construction site at the clicked position");
                        return {.entity = entity, .layer = MapLayerType::ON_GROUND};
                    }
                }
            }
        }
    }
    return result;
}
