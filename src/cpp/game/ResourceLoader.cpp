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

namespace fs = std::filesystem;
using namespace game;
using namespace aion;

ResourceLoader::ResourceLoader(std::stop_token* stopToken,
                               std::shared_ptr<GameSettings> settings,
                               GraphicsRegistry& graphicsRegistry,
                               std::shared_ptr<Renderer> renderer)
    : SubSystem(stopToken), m_settings(std::move(settings)), m_graphicsRegistry(graphicsRegistry),
      m_renderer(std::move(renderer))
{
}

void game::ResourceLoader::loadEntities()
{
    spdlog::info("Loading entities...");

    auto& gameState = GameState::getInstance();

    auto size = m_settings->getWorldSizeInTiles();
    gameState.initializeStaticEntityMap(size.width, size.height);
    gameState.generateMap();
    // gameState.generateDebugMap();

    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            auto tile = gameState.createEntity();
            gameState.addComponent(tile, CompTransform(i * 256, j * 256));
            gameState.addComponent(tile, CompRendering());
            gameState.addComponent(tile, CompEntityInfo(2, rand() % 50 + 1));
            gameState.addComponent(tile, CompDirty());

            auto map = gameState.staticEntityMap;
            map.entityMap[i][j] = tile;

            CompGraphics gc;
            gc.entityID = tile;
            gc.entityType = 2; // Tile
            gameState.addComponent(tile, gc);
        }
    }

    {
        auto villager = gameState.createEntity();
        auto transform = CompTransform(20 * 256, 20 * 256);
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

    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            if (gameState.staticEntityMap.map[i][j] == 1)
            {
                auto tree = gameState.createEntity();
                auto transform = CompTransform(i * 256 + 128, j * 256 + 128);
                transform.face(Direction::NORTHWEST);
                gameState.addComponent(tree, transform);
                gameState.addComponent(tree, CompRendering());
                CompGraphics gc;
                gc.debugOverlays.push_back({DebugOverlay::Type::CIRCLE, DebugOverlay::Color::RED,
                                            DebugOverlay::Anchor::BOTTOM_CENTER});
                gc.isStatic = true;
                gc.entityID = tree;
                gc.entityType = 4; // tree
                gameState.addComponent(tree, gc);
                gameState.addComponent(tree, CompEntityInfo(4, rand() % 10));
                gameState.addComponent(tree, CompDirty());
            }
        }
    }

    spdlog::info("Entity loaded successfully.");
}

void ResourceLoader::init()
{
    loadEntities();
}
