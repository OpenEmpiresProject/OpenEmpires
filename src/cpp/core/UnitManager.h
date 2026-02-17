#ifndef UNITMANAGER_H
#define UNITMANAGER_H

#include "EventHandler.h"
#include "utils/Types.h"

namespace core
{
class StateManager;

class UnitManager : public EventHandler
{
  public:
    UnitManager();

  private:
    void onTick(const Event& e);
    void onEntityDeletion(const Event& e);
    void onCreateUnit(const Event& e);
    void onUnitTileMovement(const Event& e);

  private:
    void handleHealths();
    LazyServiceRef<StateManager> m_stateMan;
    Ref<Player> m_nature;
};

} // namespace core

#endif