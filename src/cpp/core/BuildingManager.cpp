#include "BuildingManager.h"

#include "Coordinates.h"
#include "EntityFactory.h"
#include "GameState.h"
#include "PlayerManager.h"
#include "ServiceRegistry.h"
#include "commands/CmdBuild.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "utils/ObjectPool.h"

#include <SDL3/SDL.h>

using namespace core;

BuildingManager::BuildingManager()
{
    m_coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
    m_gameState = ServiceRegistry::getInstance().getService<GameState>();
    registerCallback(Event::Type::BUILDING_REQUESTED, this, &BuildingManager::onBuildingRequest);
    registerCallback(Event::Type::KEY_UP, this, &BuildingManager::onKeyUp);
    registerCallback(Event::Type::MOUSE_MOVE, this, &BuildingManager::onMouseMove);
    registerCallback(Event::Type::MOUSE_BTN_UP, this, &BuildingManager::onMouseButtonUp);
    registerCallback(Event::Type::UNIT_SELECTION, this, &BuildingManager::onUnitSelection);
    registerCallback(Event::Type::TICK, this, &BuildingManager::onTick);
}

void BuildingManager::onBuildingRequest(const Event& e)
{
    cancelBuilding();
    m_currentBuildingPlacement = e.getData<BuildingPlacementData>();
    m_currentBuildingPlacement.entity = createBuilding(m_currentBuildingPlacement);

    for (auto entity : m_unitSelection.selectedEntities)
    {
        auto& builder = m_gameState->getComponent<CompBuilder>(entity);
        builder.target = m_currentBuildingPlacement.entity;
    }
    publishEvent(Event::Type::BUILDING_PLACEMENT_STARTED, m_currentBuildingPlacement);
}

uint32_t BuildingManager::createBuilding(const BuildingPlacementData& request)
{
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();
    auto entity = factory->createEntity(request.entityType, 0);

    auto [transform, playerComp] = m_gameState->getComponents<CompTransform, CompPlayer>(entity);

    transform.position = request.pos;
    playerComp.player = request.player;
    return entity;
}

void BuildingManager::onKeyUp(const Event& e)
{
    SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);

    if (scancode == SDL_SCANCODE_ESCAPE)
    {
        cancelBuilding();
    }
}

void BuildingManager::onMouseMove(const Event& e)
{
    m_lastMouseScreenPos = e.getData<MouseMoveData>().screenPos;
    if (m_currentBuildingPlacement.entity != entt::null)
    {
        auto [transform, dirty, building] =
            m_gameState->getComponents<CompTransform, CompDirty, CompBuilding>(
                m_currentBuildingPlacement.entity);

        auto feet = m_coordinates->screenUnitsToFeet(m_lastMouseScreenPos);

        bool outOfMap = false;
        building.validPlacement = canPlaceBuildingAt(building, feet, outOfMap);

        if (!outOfMap)
        {
            transform.position = building.getTileSnappedPosition(feet);
        }
        dirty.markDirty(m_currentBuildingPlacement.entity);
    }
}

void BuildingManager::onMouseButtonUp(const Event& e)
{
    if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
    {
        if (m_currentBuildingPlacement.entity != entt::null)
        {
            auto [building, transform, player, info, dirty] =
                m_gameState->getComponents<CompBuilding, CompTransform, CompPlayer, CompEntityInfo,
                                           CompDirty>(m_currentBuildingPlacement.entity);

            if (building.validPlacement)
                confirmBuilding(transform, building, info, dirty);
            else
                cancelBuilding();
        }
    }
}

void BuildingManager::onUnitSelection(const Event& e)
{
    auto data = e.getData<UnitSelectionData>();
    m_unitSelection = data.selection;
}

void BuildingManager::onTick(const Event& e)
{
    for (auto entity : CompDirty::g_dirtyEntities)
    {
        if (ServiceRegistry::getInstance().getService<GameState>()->hasComponent<CompBuilding>(
                entity))
        {
            auto [transform, building, info, player] =
                m_gameState->getComponents<CompTransform, CompBuilding, CompEntityInfo, CompPlayer>(
                    entity);

            info.variation = building.getVariationByConstructionProgress();

            if (building.constructionProgress >= 100)
            {
                onCompleteBuilding(entity, building, transform, player);
                info.variation = 0;
                info.entitySubType =
                    0; // Reseting entity sub type will essentially remove construction site
            }

            spam("Progress {}, Building subtype: {}, variation {}", building.constructionProgress,
                 info.entitySubType, info.variation);

            if (building.constructionProgress > 1 && building.isInStaticMap == false)
            {
                auto& gameMap = ServiceRegistry::getInstance().getService<GameState>()->gameMap;

                for (size_t i = 0; i < building.size.value().width; i++)
                {
                    for (size_t j = 0; j < building.size.value().height; j++)
                    {
                        gameMap.addEntity(MapLayerType::STATIC,
                                          transform.position.toTile() - Tile(i, j), entity);
                        gameMap.removeEntity(MapLayerType::ON_GROUND,
                                             transform.position.toTile() - Tile(i, j), entity);
                    }
                }

                building.isInStaticMap = true;
            }
        }
    }
}

bool BuildingManager::canPlaceBuildingAt(const CompBuilding& building,
                                         const Feet& feet,
                                         bool& outOfMap)
{
    auto settings = ServiceRegistry::getInstance().getService<GameSettings>();
    auto tile = feet.toTile();
    auto staticMap = ServiceRegistry::getInstance().getService<GameState>()->gameMap.getMap(
        MapLayerType::STATIC);

    auto isValidTile = [&](const Tile& tile)
    {
        return tile.x >= 0 && tile.y >= 0 && tile.x < settings->getWorldSizeInTiles().width &&
               tile.y < settings->getWorldSizeInTiles().height;
    };

    outOfMap = false;

    for (int i = 0; i < building.size.value().width; i++)
    {
        for (int j = 0; j < building.size.value().height; j++)
        {
            if (!isValidTile({tile.x - i, tile.y - j}))
            {
                outOfMap = true;
                return false;
            }
            if (staticMap[tile.x - i][tile.y - j].isOccupied())
            {
                return false;
            }
        }
    }
    return true;
}

void BuildingManager::cancelBuilding()
{
    if (m_currentBuildingPlacement.entity != entt::null)
    {
        auto [info, dirty] = m_gameState->getComponents<CompEntityInfo, CompDirty>(
            m_currentBuildingPlacement.entity);

        info.isDestroyed = true;
        dirty.markDirty(m_currentBuildingPlacement.entity);
        publishEvent(Event::Type::BUILDING_PLACEMENT_FINISHED, m_currentBuildingPlacement);

        m_currentBuildingPlacement = BuildingPlacementData();
    }
}

void BuildingManager::confirmBuilding(CompTransform& transform,
                                      CompBuilding& building,
                                      CompEntityInfo& info,
                                      CompDirty& dirty)
{
    publishEvent(Event::Type::BUILDING_PLACEMENT_FINISHED, m_currentBuildingPlacement);

    for (auto unit : m_unitSelection.selectedEntities)
    {
        bool isABuilder = m_gameState->hasComponent<CompBuilder>(unit);
        if (isABuilder)
        {
            auto cmd = ObjectPool<CmdBuild>::acquire();
            cmd->target = m_currentBuildingPlacement.entity;

            publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, unit});
        }
    }
    building.constructionProgress = 0;
    info.entitySubType = building.constructionSiteEntitySubType;
    info.variation = building.getVariationByConstructionProgress();
    dirty.markDirty(m_currentBuildingPlacement.entity);

    auto& gameMap = ServiceRegistry::getInstance().getService<GameState>()->gameMap;

    for (size_t i = 0; i < building.size.value().width; i++)
    {
        for (size_t j = 0; j < building.size.value().height; j++)
        {
            gameMap.addEntity(MapLayerType::ON_GROUND, transform.position.toTile() - Tile(i, j),
                              m_currentBuildingPlacement.entity);
        }
    }

    // Building is permanent now, no need to track for placement
    m_currentBuildingPlacement = BuildingPlacementData();
}

void BuildingManager::onCompleteBuilding(uint32_t entity,
                                         const CompBuilding& building,
                                         const CompTransform& transform,
                                         const CompPlayer& player)
{
    player.player->getFogOfWar()->markAsExplored(transform.position.toTile().centerInFeet(),
                                                 building.size, building.lineOfSight);

    player.player->addEntity(entity);
}
