#ifndef CORE_DEBUGWINDOW_H
#define CORE_DEBUGWINDOW_H
#include "DebugHelper.h"
#include "EventHandler.h"
#include "StateManager.h"

namespace core
{
class DebugWindow : public EventHandler
{
  public:
    DebugWindow();
    ~DebugWindow();

  private:
    bool onTick(const Event& e);
    bool onMouseMove(const Event& e);
    bool onMouseButtonUp(const Event& e);
    bool onMouseButtonDown(const Event& e);
    bool onUnitSelection(const Event& e);

    void showDebugWindow();

    LazyServiceRef<StateManager> m_stateManager;
    LazyServiceRef<DebugHelper> m_debugHelper;
    EntitySelectionData m_currentEntitySelection;

    uint32_t m_selectedEntity = std::numeric_limits<uint32_t>::max();
};
} // namespace core

#endif // CORE_DEBUGWINDOW_H
