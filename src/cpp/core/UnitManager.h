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
    void onTick(const Event& e);
    void onEntityDeletion(const Event& e);
    void onCreateUnit(const Event& e);
    void onUnitTileMovement(const Event& e);
    void onUnitFormationMove(const Event& e);
    void onUnitFormationDelete(const Event& e);

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