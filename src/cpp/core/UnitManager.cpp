#include "UnitManager.h"

#include "Coordinates.h"
#include "EntityFactory.h"
#include "Event.h"
#include "PlayerFactory.h"
#include "commands/CmdDie.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompHealth.h"
#include "components/CompPlayer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"

using namespace core;

UnitManager::UnitManager()
{
    registerCallback(Event::Type::ENTITY_DELETE, this, &UnitManager::onEntityDeletion);
    registerCallback(Event::Type::UNIT_CREATION_FINISHED, this, &UnitManager::onCreateUnit);
    registerCallback(Event::Type::UNIT_TILE_MOVEMENT, this, &UnitManager::onUnitTileMovement);
    registerCallback(Event::Type::TICK, this, &UnitManager::onTick);
    registerCallback(Event::Type::UNIT_FORMATION_COMMAND_MOVE, this,
                     &UnitManager::onUnitFormationMove);
    registerCallback(Event::Type::UNIT_FORMATION_DELETE, this, &UnitManager::onUnitFormationDelete);
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
    auto& tickData = e.getData<TickData>();

    handleHealths();
    buildDensityGrid();
    handleFormations(tickData.deltaTimeMs);
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
        if (auto unitComp = m_stateMan->tryGetComponent<CompUnit>(entity))
        {
            auto& healthComp = m_stateMan->getComponent<CompHealth>(entity);

            if (healthComp.health <= 0 and not healthComp.isDead)
            {
                spdlog::debug("Unit {} died", entity);
                auto [playerComp, transform] =
                    m_stateMan->getComponents<CompPlayer, CompTransform>(entity);
                playerComp.player->removeOwnership(entity);

                auto& gameMap = m_stateMan->gameMap();
                gameMap.removeEntity(MapLayerType::UNITS, transform.position.toTile(), entity);
                gameMap.addEntity(MapLayerType::ON_GROUND, transform.position.toTile(), entity);

                auto cmd = ObjectPool<CmdDie>::acquire();
                publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});

                healthComp.isDead = true;
            }
        }
    }
}

void UnitManager::buildDensityGrid()
{
    auto& densityGrid = m_stateMan->getDensityGrid();
    m_stateMan->getDensityGrid().clear();

    m_stateMan->getEntities<CompUnit, CompTransform, CompEntityInfo>().each(
        [&](uint32_t entity, CompUnit& unit, CompTransform& transform, CompEntityInfo& info)
        {
            if (not info.isDestroyed)
            {
                densityGrid.incrementDensity(transform.position);
            }
        });
}

void UnitManager::onUnitFormationMove(const Event& e)
{
    auto formation = e.getData<UnitFormationData>().formation;
    m_formations.insert(formation);
}

void UnitManager::onUnitFormationDelete(const Event& e)
{
    auto formation = e.getData<UnitFormationData>().formation;

    m_formations.erase(formation);
}

void UnitManager::handleFormations(int deltaTimeMs)
{
    for (auto it = m_formations.begin(); it != m_formations.end();)
    {
        auto& formation = *it;

        if (formation->getState() == FormationState::REACHED)
        {
            if (formation->getControllingPlayer() == nullptr) // no actively tracked
            {
                spdlog::debug("Formation is not actively tracked by a player, no need to manage it "
                              "any longer");
                it = m_formations.erase(it);
                continue;
            }
        }
        else
        {
            auto preAnchor = formation->getAnchor();
            formation->move(deltaTimeMs);

            /*           spdlog::debug("Formation anchor moved from {} to {}", preAnchor.toString(),
                                                formation->getAnchor().toString());*/

            for (auto& slot : formation->getSlots())
            {
                auto& unitGraphics = m_stateMan->getComponent<CompGraphics>(slot.getEntityId());

                unitGraphics.debugOverlays[2].enabled = true;
                unitGraphics.debugOverlays[2].color = Color::BLUE;
                unitGraphics.debugOverlays[2].circlePixelRadius = 20;
                unitGraphics.debugOverlays[2].absolutePosition =
                    formation->getAnchor() + slot.offsetFromAnchor;
            }
        }

        ++it;
    }
    for (auto formation : m_formations)
    {
    }
}
