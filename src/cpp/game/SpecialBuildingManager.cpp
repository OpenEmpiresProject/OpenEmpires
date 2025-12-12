#include "SpecialBuildingManager.h"

#include "../core/EntityTypeRegistry.h"
#include "../core/components/CompEntityInfo.h"
#include "CompGate.h"
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
    auto& data = e.getData<LineOfSightData>();
    auto& trackerInfo = m_stateMan->getComponent<CompEntityInfo>(data.tracker);

    if (trackerInfo.entityType == m_gateEntityType)
    {
        CompGate& gate = m_stateMan->getComponent<CompGate>(data.tracker);
        if (not gate.isLocked)
        {
            spdlog::debug("Opening the gate {}", data.target);
            gate.isOpen = true;
        }
        else
        {
            spdlog::debug("Gate {} is locked, ignoring LOS enter", data.tracker);
        }
    }
}

void SpecialBuildingManager::onExitLOS(const core::Event& e)
{
    auto& data = e.getData<LineOfSightData>();
    auto& trackerInfo = m_stateMan->getComponent<CompEntityInfo>(data.tracker);

    if (trackerInfo.entityType == m_gateEntityType)
    {
        CompGate& gate = m_stateMan->getComponent<CompGate>(data.tracker);
        if (not gate.isLocked)
        {
            spdlog::debug("Closing the gate {}", data.target);
            gate.isOpen = false;
        }
        else
        {
            spdlog::debug("Gate {} is locked, ignoring LOS exit", data.tracker);
        }
    }
}

void SpecialBuildingManager::onInit(core::EventLoop& eventLoop)
{
    m_stateMan = ServiceRegistry::getInstance().getService<StateManager>();
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

        auto& building = m_stateMan->getComponent<CompBuilding>(data.entity);
        auto& passabilityMap = m_stateMan->getPassabilityMap();

        debug_assert(building.landArea.tiles.size() == 4, "Gate should not cover 4 tiles");

        passabilityMap.setTileDynamicPassability(building.landArea.tiles[1],
                                                 DynamicPassability::PASSABLE_FOR_OWNER_OR_ALLIED,
                                                 data.player->getId());
        passabilityMap.setTileDynamicPassability(building.landArea.tiles[2],
                                                 DynamicPassability::PASSABLE_FOR_OWNER_OR_ALLIED,
                                                 data.player->getId());
    }
}
