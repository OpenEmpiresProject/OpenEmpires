#include "GameState.h"

#include "PathFinderAStar.h"
#include "PathFinderBase.h"

using namespace ion;

GameState::GameState()
{
    m_pathFinder = new PathFinderAStar();
}

GameState::~GameState()
{
}

uint32_t GameState::createEntity()
{
    return m_registry.create();
}

void GameState::destroyEntity(uint32_t entity)
{
    m_entitiesToDestroy.push_back(entity);
}

void ion::GameState::destroyAllPendingEntities()
{
    for (auto entity : m_entitiesToDestroy)
    {
        m_registry.destroy(entity);
    }
    m_entitiesToDestroy.clear();
}

bool GameState::isEntityValid(uint32_t entity) const
{
    return m_registry.valid(entity);
}

void GameState::clearAll()
{
    m_registry.clear();
}