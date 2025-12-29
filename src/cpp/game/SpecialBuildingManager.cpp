#include "SpecialBuildingManager.h"

#include "CompGate.h"
#include "EntityTypeRegistry.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "logging/Logger.h"

using namespace game;
using namespace core;

SpecialBuildingManager::SpecialBuildingManager()
{
    registerCallback(Event::Type::WITHIN_LINE_OF_SIGHT, this, &SpecialBuildingManager::onEnterLOS);
    registerCallback(Event::Type::OUT_OF_LINE_OF_SIGHT, this, &SpecialBuildingManager::onExitLOS);
    registerCallback(Event::Type::BUILDING_CONSTRUCTED, this,
                     &SpecialBuildingManager::onBuildingConstructed);

    // constructor
}

SpecialBuildingManager::~SpecialBuildingManager()
{
    // destructor
}

void SpecialBuildingManager::onEnterLOS(const core::Event& e)
{
    const auto& data = e.getData<LineOfSightData>();

    if (isAGate(data.tracker))
        tryOpeningGateFor(data.tracker, data.target, true);
}

void SpecialBuildingManager::onExitLOS(const core::Event& e)
{
    const auto& data = e.getData<LineOfSightData>();

    if (isAGate(data.tracker))
        tryOpeningGateFor(data.tracker, data.target, false);
}

void SpecialBuildingManager::onInit(core::EventLoop& eventLoop)
{
    lookupEntityTypes();
}

void SpecialBuildingManager::lookupEntityTypes()
{
    if (m_gateEntityType == entt::null)
    {
        auto registry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
        m_gateEntityType = registry->getEntityType(m_gateEntityName);
    }
}

void SpecialBuildingManager::onBuildingConstructed(const core::Event& e)
{
    const BuildingConstructedData& data = e.getData<BuildingConstructedData>();
    if (data.entityType == m_gateEntityType)
    {
        spdlog::debug("Gate constructed. Adding game level components to entity {}", data.entity);

        m_stateMan->addComponent(data.entity, CompGate());

        updatePassabilityForGate(data.entity, false);
    }
}

void SpecialBuildingManager::updatePassabilityForGate(uint32_t gateEntity, bool gateIsOpen)
{
    auto [building, player] = m_stateMan->getComponents<CompBuilding, CompPlayer>(gateEntity);
    auto& passabilityMap = m_stateMan->getPassabilityMap();
    auto playerId = player.player->getId();

    debug_assert(building.landArea.tiles.size() == 4, "Gate should not cover 4 tiles");

    DynamicPassability passability = DynamicPassability::PASSABLE_FOR_ANY;
    if (not gateIsOpen)
    {
        passability = DynamicPassability::PASSABLE_FOR_OWNER_OR_ALLIED;
    }

    passabilityMap.setTileDynamicPassability(building.landArea.tiles[1], passability, playerId);
    passabilityMap.setTileDynamicPassability(building.landArea.tiles[2], passability, playerId);
}

void SpecialBuildingManager::tryOpeningGateFor(uint32_t gateEntity, uint32_t target, bool open)
{
    auto [gateComp, playerComp, gateInfo] =
        m_stateMan->getComponents<CompGate, CompPlayer, CompEntityInfo>(gateEntity);

    if (not gateComp.isLocked)
    {
        auto& targetPlayer = m_stateMan->getComponent<CompPlayer>(target);

        if (targetPlayer.player->getId() == playerComp.player->getId())
        {
            spdlog::debug("{} the gate {}", (open ? "Opening" : "Closing"), target);
            gateComp.isOpen = open;
            gateInfo.state = open ? toInt(GateStatus::OPENED) : toInt(GateStatus::CLOSED);
            updatePassabilityForGate(gateEntity, open);
            StateManager::markDirty(gateEntity);
        }
        else
        {
            spdlog::debug("Enemy at the gate {}", target);
        }
    }
    else
    {
        spdlog::debug("Gate {} is locked, ignoring LOS {}", gateEntity, (open ? "enter" : "exist"));
    }
}

bool SpecialBuildingManager::isAGate(uint32_t entity) const
{
    auto& info = m_stateMan->getComponent<CompEntityInfo>(entity);
    return info.entityType == m_gateEntityType;
}
