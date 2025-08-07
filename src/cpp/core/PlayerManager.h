#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include "EventHandler.h"
#include "utils/Types.h"

#include <unordered_map>
#include <vector>

namespace core
{
class GameState;
class Player;

class PlayerManager : public EventHandler
{
  public:
    PlayerManager();

    Ref<Player> createPlayer();
    Ref<Player> createPlayer(uint8_t id);
    Ref<Player> getPlayer(uint8_t id) const
    {
        return m_playersById.at(id);
    }

    Ref<Player> getViewingPlayer() const
    {
        return m_playersById.at(m_currentPlayer);
    }

  private:
    void onKeyUp(const Event& e);
    uint8_t getNextPlayerId() const;
    void onUnitTileMovement(const Event& e);

  private:
    std::vector<Ref<Player>> m_players;
    std::unordered_map<uint8_t, Ref<Player>> m_playersById;
    bool m_fowEnabled = true;
    Ref<GameState> m_gameState;
    uint8_t m_currentPlayer = 1;
};

} // namespace core

#endif