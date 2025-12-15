#include "UnitManager.h"

#include "Coordinates.h"
#include "EntityFactory.h"
#include "PlayerFactory.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"

using namespace core;

UnitManager::UnitManager()
{
    m_stateMan = ServiceRegistry::getInstance().getService<StateManager>();

    registerCallback(Event::Type::ENTITY_DELETE, this, &UnitManager::onUnitDeletion);
    registerCallback(Event::Type::UNIT_CREATION_FINISHED, this, &UnitManager::onCreateUnit);
    registerCallback(Event::Type::UNIT_TILE_MOVEMENT, this, &UnitManager::onUnitTileMovement);
}

void UnitManager::onUnitTileMovement(const Event& e)
{
    auto& data = e.getData<UnitTileMovementData>();
    auto [player, unit, vision] =
        m_stateMan->getComponents<CompPlayer, CompUnit, CompVision>(data.unit);

    player.player->getFogOfWar()->markAsExplored(data.positionFeet, vision.lineOfSight);
}

void UnitManager::onUnitDeletion(const Event& e)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();

    auto entity = e.getData<EntityDeleteData>().entity;
    if (entity != entt::null && stateMan->hasComponent<CompUnit>(entity))
    {
        auto [info, transform] = stateMan->getComponents<CompEntityInfo, CompTransform>(entity);
        info.isDestroyed = true;
        StateManager::markDirty(entity);
        stateMan->gameMap().removeEntity(MapLayerType::UNITS, transform.position.toTile(), entity);
    }
}

void UnitManager::onCreateUnit(const Event& e)
{
    auto& data = e.getData<UnitCreationData>();

    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto unit = factory->createEntity(data.entityType, 0);
    auto [transform, unitComp, selectible, playerComp, vision] =
        stateMan->getComponents<CompTransform, CompUnit, CompSelectible, CompPlayer, CompVision>(
            unit);

    transform.position = data.position;
    transform.face(Direction::SOUTH);
    /*auto box = getBoundingBox(m_drs, 1388, 1);
    selectible.boundingBoxes[static_cast<int>(Direction::NONE)] = box;
    selectible.selectionIndicator = {GraphicAddon::Type::ISO_CIRCLE,
                                     GraphicAddon::IsoCircle{10, Vec2(0, 0)}};*/
    playerComp.player = data.player;

    auto newTile = transform.position.toTile();
    stateMan->gameMap().addEntity(MapLayerType::UNITS, newTile, unit);
    playerComp.player->ownEntity(unit);

    data.player->getFogOfWar()->markAsExplored(transform.position, vision.lineOfSight);
}
