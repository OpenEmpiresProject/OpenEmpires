#ifndef CORE_CURSORMANAGER_H
#define CORE_CURSORMANAGER_H

#include "EventHandler.h"
#include "GraphicsRegistry.h"
#include "Property.h"
#include <unordered_map>
#include "components/CompCursor.h"

namespace core
{
class CursorManager : public EventHandler, public PropertyInitializer
{
public:
    CursorManager(const std::unordered_map<CursorType, GraphicsID>& cursors);
    ~CursorManager();


private:
    Ref<StateManager> m_stateMan;
    std::unordered_map<CursorType, GraphicsID> m_cursors;
    uint32_t m_cursorEntityId = entt::null;
    CompCursor* m_cursorComp = nullptr;
    bool m_buildingPlacementInProgress = false;
    Vec2 m_currentCursorPosition = Vec2::null;
    Vec2 m_lastCursorPosition = Vec2::null; // With respect to tick
    const float m_mouseMoveThreshold = 10;
    EntitySelectionData m_currentEntitySelectionData;

    void onMouseMove(const Event& e);
    void onInit(EventLoop& eventLoop) override;
    void onBuildingPlacementStarted(const Event& e);
    void onBuildingPlacementEnded(const Event& e);
    void onTick(const Event& e);
    void onEntitySelection(const Event& e);
};
} // namespace core

#endif // CORE_CURSORMANAGER_H
