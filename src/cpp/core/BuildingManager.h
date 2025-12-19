#ifndef BUILDINGMANAGER_H
#define BUILDINGMANAGER_H

#include "EventHandler.h"
#include "components/CompUnitFactory.h"
#include "components/CompVision.h"
#include "utils/LazyServiceRef.h"

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

    LazyServiceRef<StateManager> m_stateMan;
    LazyServiceRef<Settings> m_settings;
    LazyServiceRef<EntityTypeRegistry> m_typeRegistry;
    std::map<uint32_t /*building id*/, ActiveFactoryInfo> m_activeFactories;

  private:
    void onBuildingRequest(const Event& e);
    void onTick(const Event& e);
    void onEntityDeletion(const Event& e);
    void updateInProgressUnitCreations(auto& tick);
    void updateInProgressConstructions();
    void onQueueUnit(const Event& e);
    void onUngarrison(const Event& e);
    uint32_t createBuilding(const BuildingPlacementData& request);
    void onCompleteBuilding(uint32_t entity,
                            const CompBuilding& building,
                            const CompVision& vision,
                            const CompTransform& transform,
                            const CompPlayer& player,
                            const CompEntityInfo& info);
    Feet findVacantPositionAroundBuilding(uint32_t building);
};

} // namespace core

#endif