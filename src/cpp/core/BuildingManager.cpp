#include "BuildingManager.h"

#include "EntityFactory.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "commands/CmdBuild.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnitFactory.h"
#include "utils/ObjectPool.h"

using namespace core;

BuildingManager::BuildingManager()
{
    m_gameState = ServiceRegistry::getInstance().getService<GameState>();
    registerCallback(Event::Type::TICK, this, &BuildingManager::onTick);
    registerCallback(Event::Type::BUILDING_REQUESTED, this, &BuildingManager::onBuildingRequest);
    registerCallback(Event::Type::UNIT_QUEUE_REQUEST, this, &BuildingManager::onQueueUnit);
}

void BuildingManager::onTick(const Event& e)
{
    updateInProgressConstructions();
    updateInProgressUnitCreations(e.getData<TickData>());
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

void BuildingManager::onBuildingRequest(const Event& e)
{
    auto data = e.getData<BuildingPlacementData>();
    data.entity = createBuilding(data);

    publishEvent(Event::Type::BUILDING_APPROVED, data);
}

uint32_t BuildingManager::createBuilding(const BuildingPlacementData& request)
{
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();
    auto entity = factory->createEntity(request.entityType, 0);

    auto [transform, playerComp, building, info, dirty] =
        m_gameState
            ->getComponents<CompTransform, CompPlayer, CompBuilding, CompEntityInfo, CompDirty>(
                entity);

    transform.position = request.pos;
    playerComp.player = request.player;

    building.constructionProgress = 0;
    info.entitySubType = building.constructionSiteEntitySubType;
    info.variation = building.getVariationByConstructionProgress();
    dirty.markDirty(entity);

    auto& gameMap = m_gameState->gameMap();

    for (size_t i = 0; i < building.size.value().width; i++)
    {
        for (size_t j = 0; j < building.size.value().height; j++)
        {
            gameMap.addEntity(MapLayerType::ON_GROUND, transform.position.toTile() - Tile(i, j),
                              entity);
        }
    }
    return entity;
}

void BuildingManager::onQueueUnit(const Event& e)
{
    auto& data = e.getData<UnitQueueData>();
    spdlog::debug("On queuing unit {} for building {}", data.entityType, data.building);

    auto [building, factory] =
        m_gameState->getComponents<CompBuilding, CompUnitFactory>(data.building);
    factory.productionQueue.push_back(data.entityType);

    ActiveFactoryInfo info{.factory = factory, .building = data.building, .player = data.player};
    m_activeFactories.push_back(info);
}

Feet BuildingManager::findVacantPositionAroundBuilding(uint32_t building)
{
    auto [transform, buildingComp] =
        m_gameState->getComponents<CompTransform, CompBuilding>(building);
    auto tile = transform.position.toTile();
    auto bottomCorner = tile + 1;
    Tile pos = bottomCorner;

    auto size = buildingComp.size.value();
    auto tileMap = m_gameState->gameMap();

    // Find vacant tile through bottom left edge of the building
    for (int dx = 0; dx < size.width + 2; ++dx)
    {
        pos.x = bottomCorner.x - dx;
        if (tileMap.isOccupied(MapLayerType::UNITS, pos) == false &&
            tileMap.isOccupied(MapLayerType::STATIC, pos) == false)
        {
            return pos.centerInFeet();
        }
    }

    pos = bottomCorner;
    // Find vacant tile through bottom right edge of the building
    for (int dy = 0; dy < size.height + 2; ++dy)
    {
        pos.y = bottomCorner.y - dy;
        if (tileMap.isOccupied(MapLayerType::UNITS, pos) == false &&
            tileMap.isOccupied(MapLayerType::STATIC, pos) == false)
        {
            return pos.centerInFeet();
        }
    }
    // TODO: Need to find a vacant area somehow, or push others away
    spdlog::warn(
        "Could not find vacant area to place the unit, proceeding bottom corner for now. TODO");
    return pos.centerInFeet();
}

void BuildingManager::updateInProgressConstructions()
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
                auto& gameMap = ServiceRegistry::getInstance().getService<GameState>()->gameMap();

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

void BuildingManager::updateInProgressUnitCreations(auto& tick)
{
    for (auto it = m_activeFactories.begin(); it != m_activeFactories.end();)
    {
        auto& activeFactoryInfo = *it;
        activeFactoryInfo.factory.currentUnitProgress +=
            float(activeFactoryInfo.factory.unitCreationSpeed) * tick.deltaTimeMs / 1000.0f;

        if (activeFactoryInfo.factory.currentUnitProgress > 100)
        {
            UnitCreationData data;
            data.entityType = activeFactoryInfo.factory.productionQueue.at(0);
            data.player = activeFactoryInfo.player;
            data.position = findVacantPositionAroundBuilding(activeFactoryInfo.building);
            publishEvent(Event::Type::UNIT_CREATION_FINISHED, data);

            activeFactoryInfo.factory.currentUnitProgress = 0;
            activeFactoryInfo.factory.productionQueue.erase(
                activeFactoryInfo.factory.productionQueue.begin());
        }

        if (activeFactoryInfo.factory.productionQueue.empty())
            it = m_activeFactories.erase(it);
        else
            ++it;
    }
}
