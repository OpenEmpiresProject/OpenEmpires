#ifndef BUILDINGMANAGER_H
#define BUILDINGMANAGER_H

#include "Coordinates.h"
#include "EventHandler.h"

namespace ion
{
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

    void onMouseButtonUp(const Event& e);
    void onMouseMove(const Event& e);
    void onKeyUp(const Event& e);
    void onBuildingRequest(const Event& e);
    bool canPlaceBuildingAt(const CompBuilding& building, const Feet& feet, bool& outOfMap);
    void cancelBuilding();
    void confirmBuilding(const CompBuilding& building,
                         const CompTransform& transform,
                         const CompPlayer& player);
};

} // namespace ion

#endif