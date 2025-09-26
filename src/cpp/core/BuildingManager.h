#ifndef BUILDINGMANAGER_H
#define BUILDINGMANAGER_H

#include "EventHandler.h"
#include "components/CompUnitFactory.h"

#include <functional>
#include <map>

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
    struct ActiveFactoryInfo
    {
        std::reference_wrapper<CompUnitFactory> factory;
        uint32_t building;
        Ref<Player> player;
    };

    Ref<GameState> m_gameState;
    std::map<uint32_t /*building id*/, ActiveFactoryInfo> m_activeFactories;

  private:
    void onBuildingRequest(const Event& e);
    void onTick(const Event& e);
    void updateInProgressUnitCreations(auto& tick);
    void updateInProgressConstructions();
    void onQueueUnit(const Event& e);
    uint32_t createBuilding(const BuildingPlacementData& request);
    void onCompleteBuilding(uint32_t entity,
                            const CompBuilding& building,
                            const CompTransform& transform,
                            const CompPlayer& player);
    Feet findVacantPositionAroundBuilding(uint32_t building);
};

} // namespace core

#endif