#include "GameState.h"

#include <random>
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

GameState::StaticEntityMap GameState::initializeStaticEntityMap(int width, int height)
{
    assert(staticEntityMap.map == nullptr, "Static entity map already initialized.");

    staticEntityMap.width = width;
    staticEntityMap.height = height;
    staticEntityMap.map = new int*[width];

    for (int i = 0; i < width; ++i)
    {
        staticEntityMap.map[i] = new int[height];
        for (int j = 0; j < height; ++j)
        {
            staticEntityMap.map[i][j] = 0; // Initialize with 0 to indicate no entity
        }
    }
    return staticEntityMap;
}

GameState::StaticEntityMap aion::GameState::generateMap()
{
    assert(staticEntityMap.map != nullptr, "Static entity map is not initialized.");

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> posX(0, staticEntityMap.width - 1);
    std::uniform_int_distribution<int> posY(0, staticEntityMap.height - 1);

    int totalTiles = staticEntityMap.width * staticEntityMap.height;
    int targetTreeTiles = totalTiles / 2; // 50% of map should have trees
    int treesPlaced = 0;

    // 1. Place tree clusters (forests)
    while (treesPlaced < targetTreeTiles * 0.85) // Majority in forests
    {
        int clusterSize = 5 + rng() % 15; // Random cluster size: 5~20
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

        if (staticEntityMap.map[y][x] == 0)
        {
            staticEntityMap.map[y][x] = 1; // tree
            ++treesPlaced;
        }
    }

    return staticEntityMap;
}
