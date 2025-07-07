#include "BuildingManager.h"

#include "Coordinates.h"
#include "GameState.h"
#include "PlayerManager.h"
#include "ServiceRegistry.h"
#include "commands/CmdBuild.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompTransform.h"

#include <SDL3/SDL.h>

using namespace ion;

BuildingManager::BuildingManager(/* args */)
{
    m_coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
    registerCallback(Event::Type::BUILDING_REQUESTED, this, &BuildingManager::onBuildingRequest);
    registerCallback(Event::Type::KEY_UP, this, &BuildingManager::onKeyUp);
    registerCallback(Event::Type::MOUSE_MOVE, this, &BuildingManager::onMouseMove);
    registerCallback(Event::Type::MOUSE_BTN_UP, this, &BuildingManager::onMouseButtonUp);
    registerCallback(Event::Type::UNIT_SELECTION, this, &BuildingManager::onUnitSelection);
    registerCallback(Event::Type::TICK, this, &BuildingManager::onTick);
}

BuildingManager::~BuildingManager()
{
}

void BuildingManager::onBuildingRequest(const Event& e)
{
    cancelBuilding();
    m_currentBuildingPlacement = e.getData<BuildingPlacementData>();

    for (auto entity : m_unitSelection.selectedEntities)
    {
        auto& builder = Entity::getComponent<CompBuilder>(entity);
        builder.target = m_currentBuildingPlacement.entity;
    }
    publishEvent(Event::Type::BUILDING_PLACEMENT_STARTED, m_currentBuildingPlacement);
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
            Entity::getComponents<CompTransform, CompDirty, CompBuilding>(
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
                Entity::getComponents<CompBuilding, CompTransform, CompPlayer, CompEntityInfo,
                                      CompDirty>(m_currentBuildingPlacement.entity);

            if (building.validPlacement)
                confirmBuilding(building, info, dirty);
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
        if (GameState::getInstance().hasComponent<CompBuilding>(entity))
        {
            auto [transform, building, info, player] =
                Entity::getComponents<CompTransform, CompBuilding, CompEntityInfo, CompPlayer>(
                    entity);

            info.variation = building.getVisualVariation();

            if (building.constructionProgress >= 100)
                onCompleteBuilding(building, transform, player);

            if (building.constructionProgress > 1 && building.isInStaticMap == false)
            {
                auto& gameMap = GameState::getInstance().gameMap;

                for (size_t i = 0; i < building.size.width; i++)
                {
                    for (size_t j = 0; j < building.size.height; j++)
                    {
                        gameMap.addEntity(MapLayerType::STATIC, transform.position.toTile() - Tile(i, j),
                                        entity);
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
    auto staticMap = GameState::getInstance().gameMap.getMap(MapLayerType::STATIC);

    auto isValidTile = [&](const Tile& tile)
    {
        return tile.x >= 0 && tile.y >= 0 && tile.x < settings->getWorldSizeInTiles().width &&
               tile.y < settings->getWorldSizeInTiles().height;
    };

    outOfMap = false;

    for (int i = 0; i < building.size.width; i++)
    {
        for (int j = 0; j < building.size.height; j++)
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
        auto [info, dirty] =
            Entity::getComponents<CompEntityInfo, CompDirty>(m_currentBuildingPlacement.entity);

        info.isDestroyed = true;
        dirty.markDirty(m_currentBuildingPlacement.entity);
        publishEvent(Event::Type::BUILDING_PLACEMENT_FINISHED, m_currentBuildingPlacement);

        m_currentBuildingPlacement = BuildingPlacementData();
    }
}

void BuildingManager::confirmBuilding(CompBuilding& building,
                                      CompEntityInfo& info,
                                      CompDirty& dirty)
{
    publishEvent(Event::Type::BUILDING_PLACEMENT_FINISHED, m_currentBuildingPlacement);

    for (auto unit : m_unitSelection.selectedEntities)
    {
        bool isABuilder = Entity::hasComponent<CompBuilder>(unit);
        if (isABuilder)
        {
            auto cmd = ObjectPool<CmdBuild>::acquire();
            cmd->target = m_currentBuildingPlacement.entity;

            publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, unit});
        }
    }
    building.constructionProgress = 0;
    info.variation = building.getVisualVariation();
    dirty.markDirty(m_currentBuildingPlacement.entity);

    // Building is permanent now, no need to track for placement
    m_currentBuildingPlacement = BuildingPlacementData();
}

void BuildingManager::onCompleteBuilding(const CompBuilding& building,
                                         const CompTransform& transform,
                                         const CompPlayer& player)
{
    player.player->getFogOfWar()->markAsExplored(transform.position.toTile().centerInFeet(),
                                                 building.size, building.lineOfSight);

    player.player->addEntity(m_currentBuildingPlacement.entity);
}
