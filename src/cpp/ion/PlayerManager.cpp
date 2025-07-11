#include "PlayerManager.h"

#include "GameState.h"
#include "ServiceRegistry.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"

#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <list>

using namespace ion;

PlayerManager::PlayerManager()
{
    m_gameState = ServiceRegistry::getInstance().getService<GameState>();
    registerCallback(Event::Type::UNIT_TILE_MOVEMENT, this, &PlayerManager::onUnitTileMovement);
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

void PlayerManager::onUnitTileMovement(const Event& e)
{
    auto data = e.getData<UnitTileMovementData>();
    auto [player, unit] = m_gameState->getComponents<CompPlayer, CompUnit>(data.unit);

    player.player->getFogOfWar()->markAsExplored(data.positionFeet, unit.lineOfSight);
}