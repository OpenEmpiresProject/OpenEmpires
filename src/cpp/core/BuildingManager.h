#ifndef BUILDINGMANAGER_H
#define BUILDINGMANAGER_H

#include "EventHandler.h"

namespace core
{
class CompBuilding;
class CompTransform;
class CompPlayer;
class BuildingManager : public EventHandler
{
  public:
    BuildingManager();

  private:
    Ref<GameState> m_gameState;

  private:
    void onBuildingRequest(const Event& e);
    void onTick(const Event& e);

    uint32_t createBuilding(const BuildingPlacementData& request);
    void onCompleteBuilding(uint32_t entity,
                            const CompBuilding& building,
                            const CompTransform& transform,
                            const CompPlayer& player);
};

} // namespace core

#endif