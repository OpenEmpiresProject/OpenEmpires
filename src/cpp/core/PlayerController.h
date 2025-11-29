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
    Vec2 m_currentMouseScreenPos;

    // Building placement related
    std::unordered_map<uint32_t, BuildingPlacementData> m_currentBuildingPlacements;
    std::unordered_map<uint32_t, std::list<uint32_t>> m_entityByTypeRecyclePool;
    bool m_seriesConstructionAllowed = false;
    bool m_seriesConstructionInitiated = false;
    Tile m_firstBuildingPos = Tile::null;
    Tile m_lastMouseTile = Tile::null;

    // Garrison related
    bool m_garrisonOperationInProgress = false;

    // Selection related
    Vec2 m_selectionStartPosScreenUnits;
    bool m_isSelectionBoxInProgress = false;
    EntitySelectionData m_currentEntitySelection;

  private:
    struct ConnectedBuildingPosition
    {
        Tile pos;
        BuildingOrientation orientation;
    };

    // Event callbacks
    void onKeyUp(const Event& e);
    void onMouseButtonUp(const Event& e);
    void onMouseButtonDown(const Event& e);
    void onMouseMove(const Event& e);
    void onBuildingApproved(const Event& e);

    void resolveAction(const Vec2& screenPos);
    void createUnit(uint32_t entityType, const EntitySelection& selectedBuildings);

    // Building placement related
    void startBuildingPlacement(uint32_t buildingType);
    BuildingPlacementData createBuildingPlacement(uint32_t buildingType,
                                                  const Feet& pos,
                                                  const BuildingOrientation& orientation);
    uint32_t getOrCreateBuildingEntity(uint32_t buildingType);
    void concludeBuildingPlacement(uint32_t placementId);
    void concludeAllBuildingPlacements();
    void confirmBuildingPlacement(const BuildingPlacementData& placement) const;
    void validateAndSnapBuildingToTile(BuildingPlacementData& placement, const Feet& pos);
    void removeAllExistingBuildingPlacements();
    void calculateConnectedBuildingsPath(const Tile& start,
                                         const Tile& end,
                                         std::list<ConnectedBuildingPosition>& connectedBuildings);
    void createConnectedBuildingPlacements(
        const std::list<ConnectedBuildingPosition>& connectedBuildings, uint32_t buildingType);
    void rotateCurrentPlacement();
    void changePlacementOrientation(BuildingPlacementData& placement,
                                    BuildingOrientation orientation);

    // Garrison related
    void initiateGarrison();
    void initiateUngarrison();
    void concludeGarrison();
    void tryCompleteGarrison(uint32_t unitId, uint32_t targetBuildingEntityId);

    // Selection related
    void selectEntities(const Vec2& screenPos);
    void selectHomogeneousEntities(const std::vector<uint32_t>& selectedEntities);
    void handleClickToSelect(const Vec2& screenPos);
    void completeSelectionBox(const Vec2& startScreenPos, const Vec2& endScreenPos);
    void updateSelection(const EntitySelectionData& newSelection);
    void getAllOverlappingEntities(const Vec2& startScreenPos,
                                   const Vec2& endScreenPos,
                                   std::vector<uint32_t>& entitiesToAddToSelection);
    void onUnitSelection(const Event& e);
};
} // namespace core

#endif // CORE_PLAYERCONTROLLER_H
