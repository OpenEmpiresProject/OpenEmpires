#include "BuildingManager.h"

#include "EntityFactory.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "commands/CmdBuild.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompTransform.h"
#include "utils/ObjectPool.h"

using namespace core;

BuildingManager::BuildingManager()
{
    m_gameState = ServiceRegistry::getInstance().getService<GameState>();
    registerCallback(Event::Type::TICK, this, &BuildingManager::onTick);
    registerCallback(Event::Type::BUILDING_REQUESTED, this, &BuildingManager::onBuildingRequest);
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
