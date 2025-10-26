#ifndef CORE_CURSORMANAGER_H
#define CORE_CURSORMANAGER_H

#include "Coordinates.h"
#include "EntityTypeRegistry.h"
#include "EventHandler.h"
#include "GraphicsRegistry.h"
#include "Property.h"
#include "components/CompCursor.h"

#include <unordered_map>

namespace core
{
class CursorManager : public EventHandler, public PropertyInitializer
{
  public:
    CursorManager();

  private:
    Ref<StateManager> m_stateMan;
    Ref<Coordinates> m_coordinates;
    Ref<EntityTypeRegistry> m_registry;
    uint32_t m_cursorEntityId = entt::null;
    CompCursor* m_cursorComp = nullptr;
    bool m_buildingPlacementInProgress = false;
    Vec2 m_currentCursorPosition = Vec2::null;
    Vec2 m_lastCursorPosition = Vec2::null; // With respect to tick
    const float m_mouseMoveThreshold = 10;
    EntitySelectionData m_currentEntitySelectionData;
    bool m_cursorMovedAcrossTiles = false;
    Tile m_lastCursorTilePos;
    bool m_garrisonInprogress = false;

    void onMouseMove(const Event& e);
    void onInit(EventLoop& eventLoop) override;
    void onBuildingPlacementStarted(const Event& e);
    void onBuildingPlacementEnded(const Event& e);
    void onGarrisonRequest(const Event& e);
    void onTick(const Event& e);
    void onEntitySelection(const Event& e);
    void setCursor(const CursorType& type);
};
} // namespace core

#endif // CORE_CURSORMANAGER_H
