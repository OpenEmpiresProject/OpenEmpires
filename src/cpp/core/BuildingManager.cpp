#include "BuildingManager.h"

#include "EntityFactory.h"
#include "EntityTypeRegistry.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "commands/CmdBuild.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGarrison.h"
#include "components/CompPlayer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "components/CompUnitFactory.h"
#include "utils/ObjectPool.h"

using namespace core;

BuildingManager::BuildingManager()
{
    m_stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    m_typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
    m_settings = ServiceRegistry::getInstance().getService<Settings>();

    registerCallback(Event::Type::TICK, this, &BuildingManager::onTick);
    registerCallback(Event::Type::BUILDING_REQUESTED, this, &BuildingManager::onBuildingRequest);
    registerCallback(Event::Type::UNIT_QUEUE_REQUEST, this, &BuildingManager::onQueueUnit);
    registerCallback(Event::Type::UNGARRISON_REQUEST, this, &BuildingManager::onUngarrison);
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
    player.player->getFogOfWar()->markAsExplored(building.landArea, building.lineOfSight);
    player.player->addEntity(entity);
}

void BuildingManager::onBuildingRequest(const Event& e)
{
    auto data = e.getData<BuildingPlacementData>();
    data.entity = createBuilding(data);

    publishEvent(Event::Type::BUILDING_CREATED, data);
}

uint32_t BuildingManager::createBuilding(const BuildingPlacementData& request)
{
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();
    auto entity = factory->createEntity(request.entityType, 0);

    auto [transform, playerComp, building, info, dirty] =
        m_stateMan
            ->getComponents<CompTransform, CompPlayer, CompBuilding, CompEntityInfo, CompDirty>(
                entity);

    transform.position = request.pos;
    playerComp.player = request.player;

    building.constructionProgress = 0;
    building.orientation = request.orientation;
    building.updateLandArea(transform.position.toTile());

    info.entitySubType = building.constructionSiteEntitySubType;
    info.variation = building.getVariationByConstructionProgress();
    dirty.markDirty(entity);

    auto& gameMap = m_stateMan->gameMap();

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
        m_stateMan->getComponents<CompBuilding, CompUnitFactory>(data.building);
    factory.productionQueue.push_back(data.entityType);

    ActiveFactoryInfo info{.factory = factory, .building = data.building, .player = data.player};
    m_activeFactories.emplace(data.building, info);
}

Feet BuildingManager::findVacantPositionAroundBuilding(uint32_t building)
{
    auto [transform, buildingComp] =
        m_stateMan->getComponents<CompTransform, CompBuilding>(building);
    auto tile = transform.position.toTile();
    auto bottomCorner = tile + 1;
    Tile pos = bottomCorner;

    auto size = buildingComp.size.value();
    auto tileMap = m_stateMan->gameMap();

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
        if (ServiceRegistry::getInstance().getService<StateManager>()->hasComponent<CompBuilding>(
                entity))
        {
            auto [transform, building, info, player] =
                m_stateMan->getComponents<CompTransform, CompBuilding, CompEntityInfo, CompPlayer>(
                    entity);

            info.variation = building.getVariationByConstructionProgress();

            if (building.constructionProgress >= 100)
            {
                onCompleteBuilding(entity, building, transform, player);
                info.variation = 0;

                // Reseting entity sub type will essentially remove construction site
                info.entitySubType = 0;
            }

            spam("Progress {}, Building subtype: {}, variation {}", building.constructionProgress,
                 info.entitySubType, info.variation);

            if (building.constructionProgress > 1 && building.isInStaticMap == false)
            {
                auto& gameMap =
                    ServiceRegistry::getInstance().getService<StateManager>()->gameMap();

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
        /*
         *   Approach: Checking for sufficient housing capacity is done at
         *   the very beginning before start creating the unit and just after creating
         *   the unit but before placing it on the map.
         *   Factory will no longer be tracked if the queue is completed/empty.
         */
        auto& activeFactoryInfo = it->second;
        CompUnitFactory& factory = activeFactoryInfo.factory.get();
        factory.pausedDueToInsufficientHousing = false;
        factory.pausedDueToPopulationLimit = false;
        auto unitType = factory.productionQueue.at(0);

        if (factory.currentUnitProgress == 0)
        {
            auto newPopulation = activeFactoryInfo.player->getPopulation() +
                                 m_typeRegistry->getUnitTypeHousingNeed(unitType);
            if (newPopulation > m_settings->getMaxPopulation())
            {
                factory.pausedDueToPopulationLimit = true;
                ++it;
                continue;
            }
            if (activeFactoryInfo.player->getVacantHousingCapacity() <
                m_typeRegistry->getUnitTypeHousingNeed(unitType))
            {
                factory.pausedDueToInsufficientHousing = true;
                ++it;
                continue;
            }
        }

        factory.currentUnitProgress +=
            float(factory.unitCreationSpeed) * tick.deltaTimeMs / 1000.0f;

        if (factory.currentUnitProgress > 100)
        {
            factory.currentUnitProgress = 100; //  Cap at 100 to avoid unnecessary behaviors

            auto newPopulation = activeFactoryInfo.player->getPopulation() +
                                 m_typeRegistry->getUnitTypeHousingNeed(unitType);
            if (newPopulation > m_settings->getMaxPopulation())
            {
                factory.pausedDueToPopulationLimit = true;
                ++it;
                continue;
            }

            if (activeFactoryInfo.player->getVacantHousingCapacity() <
                m_typeRegistry->getUnitTypeHousingNeed(unitType))
            {
                factory.pausedDueToInsufficientHousing = true;
                ++it;
                continue;
            }
            UnitCreationData data;
            data.entityType = unitType;
            data.player = activeFactoryInfo.player;
            data.position = findVacantPositionAroundBuilding(activeFactoryInfo.building);
            publishEvent(Event::Type::UNIT_CREATION_FINISHED, data);

            factory.currentUnitProgress = 0;
            factory.productionQueue.erase(factory.productionQueue.begin());
        }

        if (factory.productionQueue.empty())
            it = m_activeFactories.erase(it);
        else
            ++it;
    }
}

void BuildingManager::onUngarrison(const Event& e)
{
    auto& data = e.getData<UngarrisonData>();

    auto garrisonBuilding = m_stateMan->tryGetComponent<CompGarrison>(data.building);
    if (garrisonBuilding)
    {
        for (auto& unit : garrisonBuilding->garrisonedUnits)
        {
            auto [unitComp, transform] =
                m_stateMan->getComponents<CompUnit, CompTransform>(unit.unitId);
            unitComp.isGarrisoned = false;
            m_stateMan->gameMap().addEntity(MapLayerType::UNITS, transform.getTilePosition(),
                                            unit.unitId);
        }
        garrisonBuilding->garrisonedUnits.clear();
    }
}
