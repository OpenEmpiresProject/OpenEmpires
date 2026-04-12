#ifndef CORE_CURSORMANAGER_H
#define CORE_CURSORMANAGER_H

#include "Coordinates.h"
#include "EntityTypeRegistry.h"
#include "EventHandler.h"
#include "GraphicsRegistry.h"
#include "PlayerController.h"
#include "Property.h"
#include "components/CompCursor.h"
#include "utils/LazyServiceRef.h"

#include <unordered_map>

namespace core
{
class CursorManager : public EventHandler, public PropertyInitializer
{
  public:
    CursorManager();

  private:
    LazyServiceRef<StateManager> m_stateMan;
    LazyServiceRef<Coordinates> m_coordinates;
    LazyServiceRef<EntityTypeRegistry> m_registry;
    LazyServiceRef<PlayerController> m_inputPlayerController;
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

    bool onMouseMove(const Event& e);
    void onInit(EventLoop& eventLoop) override;
    bool onBuildingPlacementStarted(const Event& e);
    bool onBuildingPlacementEnded(const Event& e);
    bool onGarrisonRequest(const Event& e);
    bool onTick(const Event& e);
    bool onEntitySelection(const Event& e);
    void setCursor(const CursorType& type);
};
} // namespace core

#endif // CORE_CURSORMANAGER_H
