#include "PlayerFactory.h"

#include "Player.h"

#include <algorithm>
#include <list>

using namespace core;

PlayerFactory::PlayerFactory()
{
}

Ref<Player> PlayerFactory::createPlayer()
{
    return createPlayer(getNextPlayerId());
}

Ref<Player> PlayerFactory::createPlayer(uint8_t id)
{
    auto player = CreateRef<Player>();
    player->init(id);
    m_players.push_back(player);
    m_playersById[id] = player;
    return player;
}

uint8_t PlayerFactory::getNextPlayerId() const
{
    std::list<uint8_t> playerIds;
    for (auto& player : m_players)
    {
        playerIds.push_back(player->getId());
    }
    playerIds.sort();

    if (playerIds.empty())
        return 0;
    else
    {
        auto last = playerIds.back();
        return ++last;
    }
}
