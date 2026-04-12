#ifndef UNITMANAGER_H
#define UNITMANAGER_H

#include "EventHandler.h"
#include "utils/Types.h"

#include <set>

namespace core
{
class StateManager;

class UnitManager : public EventHandler
{
  public:
    UnitManager();

  private:
    bool onTick(const Event& e);
    bool onEntityDeletion(const Event& e);
    bool onCreateUnit(const Event& e);
    bool onUnitTileMovement(const Event& e);
    bool onUnitFormationMove(const Event& e);
    bool onUnitFormationDelete(const Event& e);

  private:
    void handleHealths();
    void buildDensityGrid();
    void handleFormations(int deltaTimeMs);
    LazyServiceRef<StateManager> m_stateMan;
    Ref<Player> m_nature;
    std::set<Ref<BaseUnitFormation>> m_formations;
};

} // namespace core

#endif