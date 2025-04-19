#include "GameState.h"

using namespace aion;

GameState::GameState(/* args */)
{
}

GameState::~GameState()
{
}

entt::entity GameState::createEntity()
{
    return registry.create();
}

void GameState::destroyEntity(entt::entity entity)
{
    registry.destroy(entity);
}

bool GameState::isEntityValid(entt::entity entity) const
{
    return registry.valid(entity);
}

void GameState::clearAll()
{
    registry.clear();
}

void GameState::init()
{
    // Initialize the GameState subsystem
}

void GameState::shutdown()
{   
    // Shutdown the GameState subsystem
}