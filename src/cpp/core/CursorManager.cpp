#include "CursorManager.h"

#include "components/CompCursor.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompTransform.h"

using namespace core;

CursorManager::CursorManager(const std::unordered_map<CursorType, GraphicsID>& cursors)
    : m_cursors(cursors)
{

    registerCallback(Event::Type::TICK, this, &CursorManager::onTick);
    registerCallback(Event::Type::MOUSE_MOVE, this, &CursorManager::onMouseMove);
    registerCallback(Event::Type::BUILDING_PLACEMENT_STARTED, this,
                     &CursorManager::onBuildingPlacementStarted);
    registerCallback(Event::Type::BUILDING_PLACEMENT_ENDED, this,
                     &CursorManager::onBuildingPlacementEnded);
    registerCallback(Event::Type::ENTITY_SELECTION, this, &CursorManager::onEntitySelection);
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

    m_cursorEntityId = m_stateMan->createEntity();
    m_cursorComp = &(m_stateMan->addComponent<CompCursor>(m_cursorEntityId));

    GraphicsID id = m_cursors.at(CursorType::DEFAULT_INGAME);
    m_cursorComp->cursor = id;

    m_stateMan->addComponent<CompTransform>(m_cursorEntityId);

    CompEntityInfo info(id.entityType, id.entitySubType, id.variation);
    PropertyInitializer::set(info.entityId, m_cursorEntityId);

    m_stateMan->addComponent<CompEntityInfo>(m_cursorEntityId, info);
    m_stateMan->addComponent<CompGraphics>(m_cursorEntityId);
    CompDirty().markDirty(m_cursorEntityId);
}

void CursorManager::onBuildingPlacementStarted(const Event& e)
{
    GraphicsID id = m_cursors.at(CursorType::BUILD);
    m_cursorComp->cursor = id;
    CompDirty().markDirty(m_cursorEntityId);
    m_buildingPlacementInProgress = true;
}

void CursorManager::onBuildingPlacementEnded(const Event& e)
{
    GraphicsID id = m_cursors.at(CursorType::DEFAULT_INGAME);
    m_cursorComp->cursor = id;
    CompDirty().markDirty(m_cursorEntityId);
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

        // Already set the proper icon for building placement (the hammer)
        if (m_buildingPlacementInProgress)
            return;

        if (m_currentEntitySelectionData.selection.selectedEntities.empty() == false &&
            m_currentEntitySelectionData.type == EntitySelectionData::Type::UNIT)
        {
            auto queryResult = m_stateMan->whatIsAt(m_currentCursorPosition);
            if (queryResult.entity != entt::null)
            {
                // TODO: Check whether at least 1 villager present
                GraphicsID id = m_cursors.at(CursorType::ASSIGN_TASK);
                m_cursorComp->cursor = id;
                CompDirty().markDirty(m_cursorEntityId);
                return;
            }
            // TODO: Military tasks
        }
        GraphicsID id = m_cursors.at(CursorType::DEFAULT_INGAME);
        m_cursorComp->cursor = id;
        CompDirty().markDirty(m_cursorEntityId);
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
