#ifndef PLAYERMANAGER_H
#define PLAYERMANAGER_H

#include "EventHandler.h"
#include "Player.h"
#include "utils/Types.h"

#include <unordered_map>
#include <vector>

namespace ion
{
class PlayerManager : public EventHandler
{
  public:
    PlayerManager();
    ~PlayerManager();

    Ref<Player> createPlayer();
    Ref<Player> createPlayer(uint8_t id);
    Ref<Player> getPlayer(uint8_t id)
    {
        return m_playersById[id];
    }

    Ref<Player> getViewingPlayer()
    {
        // TODO: Change according to the viewing
        return m_playersById[0];
    }

  private:
    uint8_t getNextPlayerId() const;
    void onUnitTileMovement(const Event& e);

    std::vector<Ref<Player>> m_players;
    std::unordered_map<uint8_t, Ref<Player>> m_playersById;
    bool m_fowEnabled = true;
};

} // namespace ion

#endif