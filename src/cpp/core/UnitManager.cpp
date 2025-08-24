#include "UnitManager.h"

#include "Coordinates.h"
#include "EntityFactory.h"
#include "PlayerFactory.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"

using namespace core;

UnitManager::UnitManager()
{
    m_gameState = ServiceRegistry::getInstance().getService<GameState>();

    registerCallback(Event::Type::ENTITY_DELETE, this, &UnitManager::onUnitDeletion);
    registerCallback(Event::Type::UNIT_REQUESTED, this, &UnitManager::onUnitRequested);
    registerCallback(Event::Type::UNIT_TILE_MOVEMENT, this, &UnitManager::onUnitTileMovement);
}

void UnitManager::onUnitTileMovement(const Event& e)
{
    auto& data = e.getData<UnitTileMovementData>();
    auto [player, unit] = m_gameState->getComponents<CompPlayer, CompUnit>(data.unit);

    player.player->getFogOfWar()->markAsExplored(data.positionFeet, unit.lineOfSight);
}

void UnitManager::onUnitDeletion(const Event& e)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();

    auto entity = e.getData<EntityDeleteData>().entity;
    if (entity != entt::null && gameState->hasComponent<CompUnit>(entity))
    {
        auto [info, dirty, transform] =
            gameState->getComponents<CompEntityInfo, CompDirty, CompTransform>(entity);
        info.isDestroyed = true;
        dirty.markDirty(entity);
        gameState->gameMap().removeEntity(MapLayerType::UNITS, transform.position.toTile(), entity);
    }
}

void UnitManager::onUnitRequested(const Event& e)
{
    auto& data = e.getData<UnitCreationData>();

    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto unit = factory->createEntity(data.entityType, 0);
    auto [transform, unitComp, selectible, playerComp] =
        gameState->getComponents<CompTransform, CompUnit, CompSelectible, CompPlayer>(unit);

    transform.position = data.position;
    transform.face(Direction::SOUTH);
    /*auto box = getBoundingBox(m_drs, 1388, 1);
    selectible.boundingBoxes[static_cast<int>(Direction::NONE)] = box;
    selectible.selectionIndicator = {GraphicAddon::Type::ISO_CIRCLE,
                                     GraphicAddon::IsoCircle{10, Vec2(0, 0)}};*/
    playerComp.player = data.player;

    auto newTile = transform.position.toTile();
    gameState->gameMap().addEntity(MapLayerType::UNITS, newTile, unit);

    data.player->getFogOfWar()->markAsExplored(transform.position, unitComp.lineOfSight);
}
