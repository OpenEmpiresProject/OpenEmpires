#include "ResourceLoader.h"

#include "GameState.h"
#include "Renderer.h"
#include "SubSystemRegistry.h"
#include "commands/CmdIdle.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <random>

namespace fs = std::filesystem;
using namespace game;
using namespace ion;

ResourceLoader::ResourceLoader(std::stop_token* stopToken,
                               std::shared_ptr<GameSettings> settings,
                               GraphicsRegistry& graphicsRegistry,
                               std::shared_ptr<Renderer> renderer)
    : SubSystem(stopToken), m_settings(std::move(settings)), m_graphicsRegistry(graphicsRegistry),
      m_renderer(std::move(renderer))
{
}

void ResourceLoader::loadEntities()
{
    spdlog::info("Loading entities...");

    auto& gameState = GameState::getInstance();

    auto size = m_settings->getWorldSizeInTiles();

    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            auto tile = gameState.createEntity();
            gameState.addComponent(tile, CompTransform(i * 256, j * 256));
            gameState.addComponent(tile, CompRendering());
            // tc = sqrt(total_tile_frame_count)
            int tc = 10;
            // Convet our top corner based coordinate to left corner based coordinate
            int newX = size.height - j;
            int newY = i;
            // AOE2 standard tiling rule. From OpenAge documentation
            int tileVariation = (newX % tc) + ((newY % tc) * tc) + 1;
            gameState.addComponent(tile, CompEntityInfo(2, tileVariation));
            gameState.addComponent(tile, CompDirty());

            auto map = gameState.gameMap;
            map.layers[MapLayerType::GROUND].cells[i][j].addEntity(tile);

            CompGraphics gc;
            gc.entityID = tile;
            gc.entityType = 2; // Tile
            DebugOverlay overlay{DebugOverlay::Type::RHOMBUS, DebugOverlay::Color::GREY,
                                 DebugOverlay::FixedPosition::BOTTOM_CENTER};
            overlay.customPos1 = DebugOverlay::FixedPosition::CENTER_LEFT;
            overlay.customPos2 = DebugOverlay::FixedPosition::CENTER_RIGHT;
            gc.debugOverlays.push_back(overlay);

            gameState.addComponent(tile, gc);
        }
    }

    {
        auto villager = gameState.createEntity();
        auto transform = CompTransform(20 * 256 + 128, 20 * 256 + 50);
        transform.face(Direction::SOUTH);
        transform.hasRotation = true;
        transform.speed = 256;

        CompAnimation anim;
        anim.animations[0].frames = 15;
        anim.animations[0].repeatable = true;
        anim.animations[0].speed = 10;

        anim.animations[1].frames = 15;
        anim.animations[1].repeatable = true;
        anim.animations[1].speed = 15;

        gameState.addComponent(villager, transform);
        gameState.addComponent(villager, CompRendering());
        gameState.addComponent(villager, CompEntityInfo(3));
        gameState.addComponent(villager, CompAction(0));
        gameState.addComponent(villager, anim);
        gameState.addComponent(villager, CompDirty());

        // villager goes idle by default
        CompUnit unit;
        unit.commandQueue.push(ObjectPool<CmdIdle>::acquire());
        gameState.addComponent(villager, unit);

        CompGraphics gc;
        gc.entityID = villager;
        gc.entityType = 3; // Villager
        gameState.addComponent(villager, gc);
    }

    generateMap(gameState.gameMap);

    spdlog::info("Entity loaded successfully.");
}

void ResourceLoader::createTree(GridMap& map, uint32_t x, uint32_t y)
{
    auto& gameState = GameState::getInstance();

    auto tree = gameState.createEntity();
    auto transform = CompTransform(x * 256 + 128, y * 256 + 128);
    transform.face(Direction::NORTHWEST);
    gameState.addComponent(tree, transform);
    gameState.addComponent(tree, CompRendering());
    CompGraphics gc;
    gc.debugOverlays.push_back({DebugOverlay::Type::CIRCLE, DebugOverlay::Color::RED,
                                DebugOverlay::FixedPosition::BOTTOM_CENTER});
    gc.isStatic = true;
    gc.entityID = tree;
    gc.entityType = 4; // tree
    gameState.addComponent(tree, gc);
    gameState.addComponent(tree, CompEntityInfo(4, rand() % 10));
    gameState.addComponent(tree, CompDirty());

    map.layers[MapLayerType::STATIC].cells[x][y].addEntity(tree);
}

void ResourceLoader::generateMap(GridMap& gameMap)
{
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> posX(0, gameMap.width - 1);
    std::uniform_int_distribution<int> posY(0, gameMap.height - 1);

    int totalTiles = gameMap.width * gameMap.height;
    int targetTreeTiles = totalTiles / 6; // 25% of map should have trees
    int treesPlaced = 0;

    // Define the middle 5x5 area boundaries
    int midXStart = gameMap.width / 2 - 10;
    int midXEnd = gameMap.width / 2 + 10;
    int midYStart = gameMap.height / 2 - 10;
    int midYEnd = gameMap.height / 2 + 10;

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

            if (x >= 0 && x < gameMap.width && y >= 0 && y < gameMap.height)
            {
                // Skip the middle 5x5 area
                if (x >= midXStart && x <= midXEnd && y >= midYStart && y <= midYEnd)
                    continue;

                if (gameMap.layers[MapLayerType::STATIC].cells[x][y].isOccupied() == false)
                {
                    createTree(gameMap, x, y);
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

        if (gameMap.layers[MapLayerType::STATIC].cells[x][y].isOccupied() == false)
        {
            createTree(gameMap, x, y);
            ++treesPlaced;
        }
    }
}

void ResourceLoader::init()
{
    loadEntities();
}

void ResourceLoader::shutdown()
{
    // Cleanup code for resource loading
}