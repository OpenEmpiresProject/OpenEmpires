#include "BuildingManager.h"

#include "GameState.h"
#include "PlayerManager.h"
#include "ServiceRegistry.h"
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
}

BuildingManager::~BuildingManager()
{
}

void BuildingManager::onBuildingRequest(const Event& e)
{
    cancelBuilding();
    m_currentBuildingPlacement = e.getData<BuildingPlacementData>();
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
            auto [building, transform, info, dirty, player] =
                Entity::getComponents<CompBuilding, CompTransform, CompEntityInfo, CompDirty,
                                      CompPlayer>(m_currentBuildingPlacement.entity);

            if (building.validPlacement)
                confirmBuilding(building, transform, player);
            else
                cancelBuilding();
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
        m_currentBuildingPlacement = BuildingPlacementData();
    }
}

void BuildingManager::confirmBuilding(const CompBuilding& building,
                                      const CompTransform& transform,
                                      const CompPlayer& player)
{
    auto& gameMap = GameState::getInstance().gameMap;

    for (size_t i = 0; i < building.size.width; i++)
    {
        for (size_t j = 0; j < building.size.height; j++)
        {
            gameMap.addEntity(MapLayerType::STATIC, transform.position.toTile() - Tile(i, j),
                              m_currentBuildingPlacement.entity);
        }
    }

    player.player->getFogOfWar()->markAsExplored(transform.position.toTile().centerInFeet(),
                                                 building.size, building.lineOfSight);

    player.player->addEntity(m_currentBuildingPlacement.entity);

    // Building is permanent now, no need to track for placement
    m_currentBuildingPlacement = BuildingPlacementData();
}
