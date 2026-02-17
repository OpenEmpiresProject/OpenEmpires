#include "UnitManager.h"

#include "Coordinates.h"
#include "EntityFactory.h"
#include "PlayerFactory.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "components/CompHealth.h"
#include "commands/CmdDie.h"

using namespace core;

UnitManager::UnitManager()
{
    registerCallback(Event::Type::ENTITY_DELETE, this, &UnitManager::onEntityDeletion);
    registerCallback(Event::Type::UNIT_CREATION_FINISHED, this, &UnitManager::onCreateUnit);
    registerCallback(Event::Type::UNIT_TILE_MOVEMENT, this, &UnitManager::onUnitTileMovement);
    registerCallback(Event::Type::TICK, this, &UnitManager::onTick);
}

void UnitManager::onUnitTileMovement(const Event& e)
{
    auto& data = e.getData<UnitTileMovementData>();
    auto [player, unit, vision] =
        m_stateMan->getComponents<CompPlayer, CompUnit, CompVision>(data.unit);

    player.player->getFogOfWar()->markAsExplored(data.positionFeet, vision.lineOfSight);
}

void UnitManager::onEntityDeletion(const Event& e)
{
    auto entity = e.getData<EntityDeleteData>().entity;
    if (entity != entt::null && m_stateMan->hasComponent<CompUnit>(entity))
    {
        auto [info, transform, playerComp] =
            m_stateMan->getComponents<CompEntityInfo, CompTransform, CompPlayer>(entity);
        info.isDestroyed = true;
        StateManager::markDirty(entity);
        m_stateMan->gameMap().removeEntity(MapLayerType::UNITS, transform.position.toTile(),
                                           entity);
        // Could be even a corpse by now
        m_stateMan->gameMap().removeEntity(MapLayerType::ON_GROUND, transform.position.toTile(),
                                           entity);
        playerComp.player->removeOwnership(entity);
    }
}

void UnitManager::onCreateUnit(const Event& e)
{
    auto& data = e.getData<UnitCreationData>();

    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto unit = factory->createEntity(data.entityType);
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

void UnitManager::onTick(const Event& e)
{
    handleHealths();
}

void UnitManager::handleHealths()
{
    if (m_nature == nullptr)
    {
        auto playerFactory = ServiceRegistry::getInstance().getService<PlayerFactory>();
        m_nature = playerFactory->getPlayer(Constants::NATURE_PLAYER_ID);
    }

    for (auto entity : StateManager::getDirtyEntities())
    {
        if (auto healthComp = m_stateMan->tryGetComponent<CompHealth>(entity))
        {
            if (healthComp->health <= 0 and not healthComp->isDead)
            {
                spdlog::debug("Unit {} died. Transfering ownership to nature and placing on-ground",
                              entity);
                auto [playerComp, transform] = m_stateMan->getComponents<CompPlayer, CompTransform>(entity);
                playerComp.player->removeOwnership(entity);

                auto& gameMap = m_stateMan->gameMap();
                gameMap.removeEntity(MapLayerType::UNITS, transform.position.toTile(),
                                                   entity);
                gameMap.addEntity(MapLayerType::ON_GROUND, transform.position.toTile(), entity);

                auto cmd = ObjectPool<CmdDie>::acquire();
                publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});

                healthComp->isDead = true;
            }
        }
    }
}