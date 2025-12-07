#ifndef BUILDINGMANAGER_H
#define BUILDINGMANAGER_H

#include "EventHandler.h"
#include "components/CompUnitFactory.h"
#include "components/CompVision.h"

#include <functional>
#include <map>

namespace core
{
class CompBuilding;
class CompTransform;
class CompPlayer;
class EntityTypeRegistry;
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

    Ref<StateManager> m_stateMan;
    Ref<Settings> m_settings;
    Ref<EntityTypeRegistry> m_typeRegistry;
    std::map<uint32_t /*building id*/, ActiveFactoryInfo> m_activeFactories;

  private:
    void onBuildingRequest(const Event& e);
    void onTick(const Event& e);
    void updateInProgressUnitCreations(auto& tick);
    void updateInProgressConstructions();
    void onQueueUnit(const Event& e);
    void onUngarrison(const Event& e);
    uint32_t createBuilding(const BuildingPlacementData& request);
    void onCompleteBuilding(uint32_t entity,
                            const CompBuilding& building,
                            CompVision& vision,
                            const CompTransform& transform,
                            const CompPlayer& player);
    Feet findVacantPositionAroundBuilding(uint32_t building);
};

} // namespace core

#endif