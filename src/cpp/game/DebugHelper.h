#ifndef GAME_DEBUGHELPER_H
#define GAME_DEBUGHELPER_H
#include "EventHandler.h"
#include "StateManager.h"

namespace game
{
class DebugHelper : public core::EventHandler
{
  public:
    DebugHelper();
    ~DebugHelper();

  private:
    bool onTick(const core::Event& e);
    bool onKeyUp(const core::Event& e);

    core::LazyServiceRef<core::StateManager> m_stateMan;
    bool m_showDebugDetails = false;
};
} // namespace game

#endif // GAME_DEBUGHELPER_H
