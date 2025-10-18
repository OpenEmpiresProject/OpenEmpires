#ifndef CORE_PLAYERCONTROLLER_H
#define CORE_PLAYERCONTROLLER_H

#include "EntitySelection.h"
#include "EventHandler.h"
#include "ShortcutResolver.h"
#include "utils/Types.h"

namespace core
{
class StateManager;
class Player;
class Coordinates;

class PlayerController : public EventHandler
{
  public:
    PlayerController();

    void setPlayer(Ref<Player> player);
    Ref<Player> getPlayer() const;

  private:
    // Common
    Ref<Player> m_player;
    bool m_fowEnabled = true;
    Ref<StateManager> m_stateMan;
    Ref<Coordinates> m_coordinates;

    // Selection related
    Vec2 m_selectionStartPosScreenUnits;
    bool m_isSelectionBoxInProgress = false;
    EntitySelectionData m_currentEntitySelection;

    // Building placement related
    BuildingPlacementData m_currentBuildingPlacement;
    Vec2 m_currentMouseScreenPos;
    bool m_garrisonOperationInProgress = false;

  private:
    // Event callbacks
    void onKeyUp(const Event& e);
    void onMouseButtonUp(const Event& e);
    void onMouseButtonDown(const Event& e);
    void onMouseMove(const Event& e);
    void onBuildingApproved(const Event& e);

    void resolveAction(const Vec2& screenPos);

    // Building placement related
    void createBuilding(const ShortcutResolver::Action& action);
    void cancelBuildingPlacement();
    void confirmBuildingPlacement(CompTransform& transform,
                                  CompBuilding& building,
                                  CompEntityInfo& info,
                                  CompDirty& dirty) const;

    void createUnit(uint32_t entityType, const EntitySelection& selectedBuildings);
    void initiateGarrison();
    void initiateUngarrison();
    void tryCompleteGarrison(uint32_t unitId, uint32_t targetBuildingEntityId);

    // Selection related
    void selectHomogeneousEntities(const std::vector<uint32_t>& selectedEntities);
    void updateSelection(const EntitySelectionData& newSelection);
    void handleClickToSelect(const Vec2& screenPos);
    void selectEntities(const Vec2& screenPos);
    void completeSelectionBox(const Vec2& startScreenPos, const Vec2& endScreenPos);
    void getAllOverlappingEntities(const Vec2& startScreenPos,
                                   const Vec2& endScreenPos,
                                   std::vector<uint32_t>& entitiesToAddToSelection);
    void onUnitSelection(const Event& e);
};
} // namespace core

#endif // CORE_PLAYERCONTROLLER_H
