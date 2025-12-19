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
    void onEntityDeletion(const Event& e);
    void onCreateUnit(const Event& e);
    void onUnitTileMovement(const Event& e);

  private:
    LazyServiceRef<StateManager> m_stateMan;
};

} // namespace core

#endif