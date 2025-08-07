#include "PlayerManager.h"

#include "GameState.h"
#include "Player.h"
#include "ServiceRegistry.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/Logger.h"

#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <list>

using namespace core;

PlayerManager::PlayerManager()
{
    m_gameState = ServiceRegistry::getInstance().getService<GameState>();
    registerCallback(Event::Type::KEY_UP, this, &PlayerManager::onKeyUp);
    registerCallback(Event::Type::UNIT_TILE_MOVEMENT, this, &PlayerManager::onUnitTileMovement);
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

void PlayerManager::onUnitTileMovement(const Event& e)
{
    auto data = e.getData<UnitTileMovementData>();
    auto [player, unit] = m_gameState->getComponents<CompPlayer, CompUnit>(data.unit);

    player.player->getFogOfWar()->markAsExplored(data.positionFeet, unit.lineOfSight);
}

void PlayerManager::onKeyUp(const Event& e)
{
    SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);

    if (scancode == SDL_SCANCODE_1)
    {
        spdlog::info("Switching to player 1");
        m_currentPlayer = 1;
    }
    else if (scancode == SDL_SCANCODE_2)
    {
        spdlog::info("Switching to player 2");
        m_currentPlayer = 2;
    }
}
