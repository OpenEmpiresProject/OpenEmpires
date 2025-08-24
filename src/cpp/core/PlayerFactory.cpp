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

    // Using 1 based IDs for player to 1) allow using zero for default value 2) align with original
    // game
    uint8_t nextId = 1;
    for (auto id : playerIds)
    {
        if (id != nextId)
        {
            return nextId;
        }
        nextId++;
    }
    return nextId;
}
