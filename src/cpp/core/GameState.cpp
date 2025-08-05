#include "GameState.h"

#include "PathFinderAStar.h"
#include "PathFinderBase.h"

using namespace core;

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

void core::GameState::destroyAllPendingEntities()
{
    for (auto entity : m_entitiesToDestroy)
    {
        m_registry.destroy(entity);
    }
    m_entitiesToDestroy.clear();
}

bool GameState::isEntityValid(uint32_t entity) const
{
    return m_registry.valid(entity) &&
           std::find(m_entitiesToDestroy.begin(), m_entitiesToDestroy.end(), entity) ==
               m_entitiesToDestroy.end();
}

void GameState::clearAll()
{
    m_registry.clear();
}