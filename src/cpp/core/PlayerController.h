#ifndef CORE_PLAYERCONTROLLER_H
#define CORE_PLAYERCONTROLLER_H

#include "EntitySelection.h"
#include "EventHandler.h"
#include "utils/Types.h"

namespace core
{
class GameState;
class Player;
class Coordinates;

class PlayerController : public EventHandler
{
  public:
    PlayerController();
    ~PlayerController();

    void setPlayer(Ref<Player> player);
    Ref<Player> getPlayer() const
    {
        return m_player;
    }

  private:
    Ref<Player> m_player;

    bool m_fowEnabled = true;
    Ref<GameState> m_gameState;
    Ref<Coordinates> m_coordinates;

    bool m_buildingPlacementInProgress = false;
    Vec2 m_selectionStartPosScreenUnits;
    bool m_isSelectionBoxInProgress = false;
    EntitySelection m_currentUnitSelection;

  private:
    void onKeyUp(const Event& e);
    void onMouseButtonUp(const Event& e);
    void onMouseButtonDown(const Event& e);
    void onBuildingPlacementStarted(const Event& e);
    void onBuildingPlacementFinished(const Event& e);
    void trySelectingEntities(const std::vector<uint32_t>& selectedEntities);
    void updateSelection(const EntitySelection& newSelection);
    void handleClickToSelect(const Vec2& screenPos);
    void handleEntitySelection(const Vec2& screenPos);
    void resolveAction(const Vec2& screenPos);
    void completeSelectionBox(const Vec2& startScreenPos, const Vec2& endScreenPos);
    void onUnitSelection(const Event& e);
};
} // namespace core

#endif // CORE_PLAYERCONTROLLER_H
