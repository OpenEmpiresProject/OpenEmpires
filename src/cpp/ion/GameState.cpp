#include "GameState.h"

#include "PathFinderAStar.h"

#include <random>
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

StaticEntityMap GameState::initializeStaticEntityMap(int width, int height)
{
    assert(staticEntityMap.map == nullptr, "Static entity map already initialized.");

    staticEntityMap.width = width;
    staticEntityMap.height = height;
    staticEntityMap.map = new int*[width];
    staticEntityMap.entityMap = new uint32_t*[width];

    for (int i = 0; i < width; ++i)
    {
        staticEntityMap.map[i] = new int[height];
        staticEntityMap.entityMap[i] = new uint32_t[height]; // Initialize entity map

        for (int j = 0; j < height; ++j)
        {
            staticEntityMap.map[i][j] = 0;                // Initialize with 0 to indicate no entity
            staticEntityMap.entityMap[i][j] = entt::null; // Initialize with null entity
        }
    }
    return staticEntityMap;
}

StaticEntityMap GameState::generateMap()
{
    assert(staticEntityMap.map != nullptr, "Static entity map is not initialized.");

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> posX(0, staticEntityMap.width - 1);
    std::uniform_int_distribution<int> posY(0, staticEntityMap.height - 1);

    int totalTiles = staticEntityMap.width * staticEntityMap.height;
    int targetTreeTiles = totalTiles / 6; // 25% of map should have trees
    int treesPlaced = 0;

    // Define the middle 5x5 area boundaries
    int midXStart = staticEntityMap.width / 2 - 10;
    int midXEnd = staticEntityMap.width / 2 + 10;
    int midYStart = staticEntityMap.height / 2 - 10;
    int midYEnd = staticEntityMap.height / 2 + 10;

    // 1. Place tree clusters (forests)
    while (treesPlaced < targetTreeTiles * 0.95) // Majority in forests
    {
        int clusterSize = 5 + rng() % 20; // Random cluster size: 5~25
        int centerX = posX(rng);
        int centerY = posY(rng);

        for (int i = 0; i < clusterSize; ++i)
        {
            int dx = (rng() % 5) - 2; // Random offset -2..+2
            int dy = (rng() % 5) - 2;

            int x = centerX + dx;
            int y = centerY + dy;

            if (x >= 0 && x < staticEntityMap.width && y >= 0 && y < staticEntityMap.height)
            {
                // Skip the middle 5x5 area
                if (x >= midXStart && x <= midXEnd && y >= midYStart && y <= midYEnd)
                    continue;

                if (staticEntityMap.map[y][x] == 0)
                {
                    staticEntityMap.map[y][x] = 1; // tree
                    ++treesPlaced;
                }
            }
        }
    }

    // 2. Place isolated trees
    while (treesPlaced < targetTreeTiles)
    {
        int x = posX(rng);
        int y = posY(rng);

        // Skip the middle 5x5 area
        if (x >= midXStart && x <= midXEnd && y >= midYStart && y <= midYEnd)
            continue;

        if (staticEntityMap.map[y][x] == 0)
        {
            staticEntityMap.map[y][x] = 1; // tree
            ++treesPlaced;
        }
    }

    return staticEntityMap;
}

StaticEntityMap GameState::generateDebugMap()
{
    for (size_t i = 0; i < 5; i++)
    {
        for (size_t j = 0; j < 1; j++)
        {
            staticEntityMap.map[i + 10][j + 10] = 1;
        }
    }
    staticEntityMap.map[15][9] = 1;

    return staticEntityMap;
}
