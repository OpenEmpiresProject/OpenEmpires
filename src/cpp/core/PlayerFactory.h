#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include "utils/Types.h"

#include <unordered_map>
#include <vector>

namespace core
{
class Player;

class PlayerFactory
{
  public:
    PlayerFactory();

    Ref<Player> createPlayer();
    Ref<Player> createPlayer(uint8_t id);
    Ref<Player> getPlayer(uint8_t id) const
    {
        return m_playersById.at(id);
    }

  private:
    uint8_t getNextPlayerId() const;

  private:
    std::vector<Ref<Player>> m_players;
    std::unordered_map<uint8_t, Ref<Player>> m_playersById;
};

} // namespace core

#endif