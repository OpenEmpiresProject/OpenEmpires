#include "PlayerManager.h"

#include <algorithm>
#include <list>

using namespace ion;

PlayerManager::PlayerManager()
{
    registerCallback(Event::Type::TICK, this, &PlayerManager::onTick);
}

PlayerManager::~PlayerManager()
{
}

Ref<Player> PlayerManager::createPlayer()
{
    return createPlayer(getNextPlayerId());
}

Ref<Player> PlayerManager::createPlayer(uint8_t id)
{
    auto player = CreateRef<Player>();
    player->init(id);
    m_players.push_back(player);
    m_playersById[id] = player;
    return player;
}

uint8_t PlayerManager::getNextPlayerId() const
{
    std::list<uint8_t> playerIds;
    for (auto player : m_players)
    {
        playerIds.push_back(player->getId());
    }
    playerIds.sort();

    uint8_t nextId = 0;
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

void PlayerManager::onEvent(const Event& e)
{
}

void PlayerManager::onTick(const Event& e)
{
}