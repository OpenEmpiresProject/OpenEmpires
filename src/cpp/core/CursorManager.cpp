#include "CursorManager.h"

#include "components/CompCursor.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompTransform.h"

using namespace core;

CursorManager::CursorManager()
{

    registerCallback(Event::Type::TICK, this, &CursorManager::onTick);
    registerCallback(Event::Type::MOUSE_MOVE, this, &CursorManager::onMouseMove);
    registerCallback(Event::Type::BUILDING_PLACEMENT_STARTED, this,
                     &CursorManager::onBuildingPlacementStarted);
    registerCallback(Event::Type::BUILDING_PLACEMENT_ENDED, this,
                     &CursorManager::onBuildingPlacementEnded);
    registerCallback(Event::Type::ENTITY_SELECTION, this, &CursorManager::onEntitySelection);
    registerCallback(Event::Type::GARRISON_REQUEST, this, &CursorManager::onGarrisonRequest);
}

void CursorManager::onMouseMove(const Event& e)
{
    auto& data = e.getData<MouseMoveData>();
    m_currentCursorPosition = data.screenPos;
}

void CursorManager::onInit(EventLoop& eventLoop)
{
    m_stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    m_coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
    m_registry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

    m_cursorEntityId = m_stateMan->createEntity();
    m_cursorComp = &(m_stateMan->addComponent<CompCursor>(m_cursorEntityId));

    m_cursorComp->cursor = m_registry->getCursorGraphic(CursorType::DEFAULT_INGAME);

    m_stateMan->addComponent<CompTransform>(m_cursorEntityId);

    CompEntityInfo info(m_cursorComp->cursor.entityType, m_cursorComp->cursor.entitySubType,
                        m_cursorComp->cursor.variation);
    PropertyInitializer::set(info.entityId, m_cursorEntityId);

    m_stateMan->addComponent<CompEntityInfo>(m_cursorEntityId, info);
    m_stateMan->addComponent<CompGraphics>(m_cursorEntityId);
    CompDirty().markDirty(m_cursorEntityId);
}

void CursorManager::onBuildingPlacementStarted(const Event& e)
{
    setCursor(CursorType::BUILD);
    m_buildingPlacementInProgress = true;
}

void CursorManager::onBuildingPlacementEnded(const Event& e)
{
    setCursor(CursorType::DEFAULT_INGAME);

    m_buildingPlacementInProgress = false;
}

/*
 * Approach: When cursor is still (i.e. not moved between two ticks) check what is under the
 * cursor and change cursor according to that and current entity selection. When the cursor is
 * still, re-perform above only if the cursor changed the tile.
 *
 * This approach prevents unnecessary,
 *      1) cursor changes when it is on the move
 *      2) cursor changes within a tile
 */
void CursorManager::onTick(const Event& e)
{
    auto movement = m_currentCursorPosition.distanceSquared(m_lastCursorPosition);
    m_lastCursorPosition = m_currentCursorPosition;
    const bool cursorIsStill = movement < m_mouseMoveThreshold;

    if (cursorIsStill)
    {
        // Cursor is still in the same tile, so no point of changing the cursor
        if (m_cursorMovedAcrossTiles == false)
            return;

        m_cursorMovedAcrossTiles = false;

        // For building placement and garrison, cursor is already set the proper icon
        // without waiting for cursor movement

        if (m_buildingPlacementInProgress or m_garrisonInprogress)
            return;

        if (m_currentEntitySelectionData.selection.selectedEntities.empty() == false and
            m_currentEntitySelectionData.type == EntitySelectionData::Type::UNIT)
        {
            auto queryResult = m_stateMan->whatIsAt(m_currentCursorPosition);
            if (queryResult.entity != entt::null)
            {
                // TODO: Check whether at least 1 villager present
                setCursor(CursorType::ASSIGN_TASK);
                return;
            }
            // TODO: Military tasks
        }
        setCursor(CursorType::DEFAULT_INGAME);
    }
    else
    {
        auto currentTile = m_coordinates->screenUnitsToTiles(m_currentCursorPosition);

        // Cursor has crossed a tile
        if (m_lastCursorTilePos != currentTile)
        {
            m_lastCursorTilePos = currentTile;
            m_cursorMovedAcrossTiles = true;
        }
    }
}

void CursorManager::onEntitySelection(const Event& e)
{
    m_currentEntitySelectionData = e.getData<EntitySelectionData>();
}

void CursorManager::onGarrisonRequest(const Event& e)
{
    auto& data = e.getData<GarrisonData>();
    m_garrisonInprogress = data.inprogress;

    setCursor(m_garrisonInprogress ? CursorType::GARRISON : CursorType::DEFAULT_INGAME);
}

void CursorManager::setCursor(const CursorType& type)
{
    m_cursorComp->cursor = m_registry->getCursorGraphic(type);
    CompDirty().markDirty(m_cursorEntityId);
}
