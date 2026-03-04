#include "BuildingManager.h"

#include "EntityFactory.h"
#include "EntityTypeRegistry.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "commands/CmdBuild.h"
#include "components/CompAnimation.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompGarrison.h"
#include "components/CompHealth.h"
#include "components/CompPlayer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "components/CompUnitFactory.h"
#include "utils/ObjectPool.h"

using namespace core;

BuildingManager::BuildingManager()
{
    registerCallback(Event::Type::TICK, this, &BuildingManager::onTick);
    registerCallback(Event::Type::BUILDING_REQUESTED, this, &BuildingManager::onBuildingRequest);
    registerCallback(Event::Type::UNIT_QUEUE_REQUEST, this, &BuildingManager::onQueueUnit);
    registerCallback(Event::Type::UNGARRISON_REQUEST, this, &BuildingManager::onUngarrison);
    registerCallback(Event::Type::ENTITY_DELETE, this, &BuildingManager::onEntityDeletion);
}

void BuildingManager::onTick(const Event& e)
{
    handleBuildingUpdates(e.getData<TickData>());
    handleBuildingDamages(e.getData<TickData>());
    updateInProgressUnitCreations(e.getData<TickData>());
}

void BuildingManager::onCompleteBuilding(uint32_t entity,
                                         const CompBuilding& building,
                                         const CompVision& vision,
                                         const CompTransform& transform,
                                         const CompPlayer& player,
                                         const CompEntityInfo& info)
{
    player.player->getFogOfWar()->markAsExplored(building.landArea, vision.lineOfSight);
    player.player->ownEntity(entity);

    BuildingConstructedData constructedData;
    constructedData.entity = entity;
    constructedData.player = player.player;
    constructedData.entityType = info.entityType;
    constructedData.pos = transform.position;
    publishEvent(Event::Type::BUILDING_CONSTRUCTED, constructedData);

    if (vision.activeTracking)
    {
        TrackingRequestData data;
        data.entity = entity;
        data.landArea = building.landArea;
        data.center = transform.position;
        publishEvent(Event::Type::TRACKING_REQUEST, data);
    }
}

void BuildingManager::onBuildingRequest(const Event& e)
{
    auto data = e.getData<BuildingPlacementData>();
    auto [info, building] = createBuilding(data);

    if (isBuildingRequestValid(building))
    {
        makeBuildingPermanent(info.entityId);

        data.entity = info.entityId;
        publishEvent(Event::Type::BUILDING_CREATED, data);
    }
    else
    {
        spdlog::debug("Cannot place the building. Ignoring the request {}", data.toString());
    }
}

std::tuple<CompEntityInfo&, CompBuilding&> BuildingManager::createBuilding(
    const BuildingPlacementData& request)
{
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();
    auto entity = factory->createEntity(request.entityType);

    auto [transform, playerComp, building, info] =
        m_stateMan->getComponents<CompTransform, CompPlayer, CompBuilding, CompEntityInfo>(entity);

    // Don't trust the arbitrary feet position, snap it.
    transform.position = building.getSnappedBuildingCenter(request.pos); // Snap
    playerComp.player = request.player;

    building.orientation = request.orientation;
    building.updateLandArea(transform.position);

    info.variation = building.getVariationByConstructionProgress();
    info.isDestroyed = true; // This building will be abandoned if placement test comes false

    return {info, building};
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

void BuildingManager::updateInProgressUnitCreations(const TickData& tick)
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

void BuildingManager::onEntityDeletion(const Event& e)
{
    auto entity = e.getData<EntityDeleteData>().entity;
    if (entity != entt::null && m_stateMan->hasComponent<CompBuilding>(entity))
    {
        deleteBuilding(entity);
    }
}

bool BuildingManager::isBuildingRequestValid(const CompBuilding& building) const
{
    bool outOfMap = false;
    bool canPlace = m_stateMan->canPlaceBuildingAt(building.landArea, outOfMap);
    return canPlace and not outOfMap;
}

void BuildingManager::makeBuildingPermanent(uint32_t entity)
{
    auto [playerComp, building, info] =
        m_stateMan->getComponents<CompPlayer, CompBuilding, CompEntityInfo>(entity);

    info.isDestroyed = false;
    StateManager::markDirty(entity);

    auto& gameMap = m_stateMan->gameMap();
    gameMap.addEntity(MapLayerType::ON_GROUND, building.landArea, entity);

    playerComp.player->ownEntity(entity);
}

void BuildingManager::handleBuildingUpdates(const TickData& tick)
{
    for (auto entity : StateManager::getDirtyEntities())
    {
        if (auto building = m_stateMan->tryGetComponent<CompBuilding>(entity))
        {
            handleConstructionProgress(tick, *building, entity);
            detectDamagedBuildings(tick, *building, entity);
        }
    }
}

void BuildingManager::handleConstructionProgress(const TickData& tick,
                                                 CompBuilding& building,
                                                 uint32_t entity)
{
    if (building.isConstructed())
        return;

    auto [transform, info, player, vision] =
        m_stateMan->getComponents<CompTransform, CompEntityInfo, CompPlayer, CompVision>(entity);

    info.variation = building.getVariationByConstructionProgress();
    const auto constructionProgress = building.getConstructionProgress();

    spam("Progress {}, variation {}", constructionProgress, info.variation);

    if (constructionProgress >= 100)
    {
        info.variation = 0; // Switch to main building variation. TODO: Shouldn't hardcode this
        building.markAsCompleted();

        onCompleteBuilding(entity, building, vision, transform, player, info);
    }

    if (constructionProgress > 1 && building.isInStaticMap == false)
    {
        auto& gameMap = m_stateMan->gameMap();
        auto& passabilityMap = m_stateMan->getPassabilityMap();

        for (auto& tile : building.landArea.tiles)
        {
            gameMap.addEntity(MapLayerType::STATIC, tile, entity);
            gameMap.removeEntity(MapLayerType::ON_GROUND, tile, entity);
            passabilityMap.setTileDynamicPassability(tile, DynamicPassability::BLOCKED_FOR_ANY,
                                                     player.player->getId());
        }

        building.isInStaticMap = true;
    }
}

// We track damaged buildings in a separate set to avoid doing heavy health checks for all buildings
// every tick. This is because we want to animate fire for damaged buildings even without the
// building is marked as dirty.
//
void BuildingManager::detectDamagedBuildings(const TickData& tick,
                                             CompBuilding& building,
                                             uint32_t entity)
{
    auto [transform, info, player, vision, health] =
        m_stateMan
            ->getComponents<CompTransform, CompEntityInfo, CompPlayer, CompVision, CompHealth>(
                entity);

    if (health.health < health.maxHealth.value())
    {
        m_damagedBuildings.insert(entity);
    }
}

void BuildingManager::handleBuildingDamages(const TickData& tick)
{
    for (auto entity : m_damagedBuildings)
    {
        if (auto building = m_stateMan->tryGetComponent<CompBuilding>(entity))
        {
            handleBuildingDamage(tick, *building, entity);
        }
    }
}

void BuildingManager::handleBuildingDamage(const TickData& tick,
                                           CompBuilding& building,
                                           uint32_t entity)
{
    if (building.isConstructing())
        return;

    auto [healthComp, info] = m_stateMan->getComponents<CompHealth, CompEntityInfo>(entity);
    FlameLevel flameLevel = FlameLevel::SMALL;
    auto healthPerc = healthComp.health / healthComp.maxHealth.value() * 100.0;

    spdlog::debug("Building health is {}/{}", healthComp.health, healthComp.maxHealth.value());

    if (healthComp.health <= 0)
    {
        if (not healthComp.isDead)
        {
            spdlog::debug("Building {} destroyed", entity);
            healthComp.isDead = true;
            Event event(Event::Type::ENTITY_DELETE, EntityDeleteData{entity});
            publishEvent(event);
            return;
        }
    }
    else if (healthPerc < 25)
    {
        flameLevel = FlameLevel::LARGE;
    }
    else if (healthPerc < 50)
    {
        flameLevel = FlameLevel::MEDIUM;
    }
    else if (healthPerc < 75)
    {
        flameLevel = FlameLevel::SMALL;
    }
    else
    {
        return; // No fire if health is above 75%
    }

    // TODO: So far we only have fire as child entities, hence this works.
    if (info.getChildEntities().empty())
    {
        auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

        for (auto& fireAnchor : building.fireAnchors.value())
        {
            auto fire = factory->createEntity(building.fireEntityType);
            auto [fireInfo, fireTransform] =
                m_stateMan->getComponents<CompEntityInfo, CompTransform>(fire);
            info.addChild(fireInfo);
            // Fire would not have an absolute position, its position is determined by the
            // building's position + fire anchor
            //
            fireTransform.position = Feet::null;
            fireTransform.relativePixelPosition = fireAnchor;
        }
    }

    for (auto fireEntity : info.getChildEntities())
    {
        auto [fireAnimComp, fireInfo] =
            m_stateMan->getComponents<CompAnimation, CompEntityInfo>(fireEntity);
        fireInfo.state = flameLevel;
        const auto& fireAnim = fireAnimComp.animations[0];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / fireAnim.speed;
        if (tick.currentTick % (int) ticksPerFrame == 0)
        {
            fireAnimComp.frame++;
            fireAnimComp.frame %= fireAnim.frames;

            StateManager::markDirty(fireEntity);
        }
    }
}

void BuildingManager::deleteBuilding(uint32_t entity)
{
    auto [info, transform, building, playerComp] =
        m_stateMan->getComponents<CompEntityInfo, CompTransform, CompBuilding, CompPlayer>(entity);
    info.isDestroyed = true;
    StateManager::markDirty(entity);

    m_stateMan->gameMap().removeEntity(building.getMapLayerType(), building.landArea, entity);
    playerComp.player->removeOwnership(entity);

    m_damagedBuildings.erase(entity);
}
