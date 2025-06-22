#include "PlayerManager.h"

#include "GameState.h"
#include "components/CompPlayer.h"
#include "components/CompUnit.h"
#include "components/CompBuilding.h"

#include <SDL3/SDL_scancode.h>
#include <algorithm>
#include <list>

using namespace ion;

PlayerManager::PlayerManager()
{
    registerCallback(Event::Type::UNIT_TILE_MOVEMENT, this, &PlayerManager::onUnitTileMovement);
    registerCallback(Event::Type::BUILDING_PLACED, this, &PlayerManager::onBuildingPlaced);
    registerCallback(Event::Type::KEY_UP, this, &PlayerManager::onToggleFOW);
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
    auto [player, unit] = GameState::getInstance().getComponents<CompPlayer, CompUnit>(data.unit);

    player.player->getFOW()->markAsExplored(data.positionFeet, unit.lineOfSight);
}

void PlayerManager::onBuildingPlaced(const Event& e)
{
    auto data = e.getData<BuildingPlacedData>();
    auto [player, building] = GameState::getInstance().getComponents<CompPlayer, CompBuilding>(data.building);

    player.player->getFOW()->markAsExplored(data.tile.centerInFeet(), building.size, building.lineOfSight);
}

void PlayerManager::onToggleFOW(const Event& e)
{
    auto data = e.getData<KeyboardData>();
    if (data.keyCode == SDL_Scancode::SDL_SCANCODE_3)
    {
        m_fowEnabled = !m_fowEnabled;
        if (m_fowEnabled)
        {
            m_players[0]->getFOW()->enable();
        }
        else
        {
            m_players[0]->getFOW()->disable();
        }
    }
}
