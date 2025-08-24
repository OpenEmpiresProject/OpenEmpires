#ifndef UNITMANAGER_H
#define UNITMANAGER_H

#include "EventHandler.h"
#include "utils/Types.h"

namespace core
{
class GameState;

class UnitManager : public EventHandler
{
  public:
    UnitManager();

  private:
    void onUnitDeletion(const Event& e);
    void onUnitRequested(const Event& e);
    void onUnitTileMovement(const Event& e);

  private:
    Ref<GameState> m_gameState;
};

} // namespace core

#endif