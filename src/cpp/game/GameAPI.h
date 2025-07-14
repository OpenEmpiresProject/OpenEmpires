#ifndef GAMEAPI_H
#define GAMEAPI_H

#include "Feet.h"
#include "utils/Types.h"

#include <cstdint>
#include <functional>
#include <list>
#include <memory>

namespace ion
{
class Player;
class Feet;
} // namespace ion

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

    GameAPI()
    {
    }
    GameAPI(std::shared_ptr<Synchronizer> synchronizer) : m_sync(std::move(synchronizer))
    {
    }

    bool isReady();
    void quit();
    ion::Ref<ion::Player> getPrimaryPlayer();
    uint32_t createVillager(ion::Ref<ion::Player>, const ion::Feet& pos);
    std::list<uint32_t> getVillagers();
    void commandToMove(uint32_t unit, const ion::Feet& target);
    int getCurrentAction(uint32_t unit);
    ion::Feet getUnitPosition(uint32_t unit);
    void deleteEntity(uint32_t entity);

  private:
    std::shared_ptr<Synchronizer> m_sync;
};

} // namespace game

#endif