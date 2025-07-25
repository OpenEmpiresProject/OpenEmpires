#ifndef BUILDINGMANAGER_H
#define BUILDINGMANAGER_H

#include "EventHandler.h"

namespace ion
{
class Coordinates;
class CompBuilding;
class CompTransform;
class CompPlayer;
class BuildingManager : public EventHandler
{
  public:
    BuildingManager(/* args */);
    ~BuildingManager();

  private:
    BuildingPlacementData m_currentBuildingPlacement;
    Vec2 m_lastMouseScreenPos;
    Ref<Coordinates> m_coordinates;
    Ref<GameState> m_gameState;
    UnitSelection m_unitSelection;

    void onMouseButtonUp(const Event& e);
    void onMouseMove(const Event& e);
    void onKeyUp(const Event& e);
    void onBuildingRequest(const Event& e);
    uint32_t createBuilding(const BuildingPlacementData& request);
    void onTick(const Event& e);
    void onUnitSelection(const Event& e);
    bool canPlaceBuildingAt(const CompBuilding& building, const Feet& feet, bool& outOfMap);
    void cancelBuilding();
    void confirmBuilding(CompTransform& transform,
                         CompBuilding& building,
                         CompEntityInfo& info,
                         CompDirty& dirty);
    void onCompleteBuilding(uint32_t entity,
                            const CompBuilding& building,
                            const CompTransform& transform,
                            const CompPlayer& player);
};

} // namespace ion

#endif