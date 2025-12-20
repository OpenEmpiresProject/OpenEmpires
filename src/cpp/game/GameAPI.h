#ifndef GAMEAPI_H
#define GAMEAPI_H

#include "Feet.h"
#include "utils/Types.h"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <unordered_set>

namespace core
{
class Player;
class Feet;
} // namespace core

namespace game
{
class GameAPI
{
  public:
    struct Synchronizer
    {
        std::function<void()> onStart;
        std::function<void()> onEnd;
    };

    GameAPI() = default;
    GameAPI(std::shared_ptr<Synchronizer> synchronizer) : m_sync(std::move(synchronizer))
    {
    }

    bool isReady();
    void quit();
    core::Ref<core::Player> getPrimaryPlayer();
    uint32_t createVillager(core::Ref<core::Player>, const core::Feet& pos);
    std::list<uint32_t> getVillagers();
    void commandToMove(uint32_t unit, const core::Feet& target);
    void placeBuilding(
        uint32_t playerId,
        int buildingType,
        const core::Feet& pos,
        core::BuildingOrientation orientation = core::BuildingOrientation::NO_ORIENTATION);
    void build(uint32_t unit, uint32_t target);
    int getCurrentAction(uint32_t unit);
    core::Feet getUnitPosition(uint32_t unit);
    void deleteEntity(uint32_t entity);
    const std::unordered_set<uint32_t>& getBuildings(uint32_t playerId);
    const std::unordered_set<uint32_t>& getConstructionSites(uint32_t playerId);
    void placeWall(uint32_t playerId,
                   int wallEntityType,
                   const core::Feet& from,
                   const core::Feet& to);

    void executeCustomSynchronizedAction(std::function<void()> func);

  private:
    std::shared_ptr<Synchronizer> m_sync;
};

} // namespace game

#endif