#include "ResourceLoader.h"

#include "GameState.h"
#include "Renderer.h"
#include "SubSystemRegistry.h"
#include "commands/CmdIdle.h"
#include "components/ActionComponent.h"
#include "components/AnimationComponent.h"
#include "components/CompUnit.h"
#include "components/DirtyComponent.h"
#include "components/EntityInfoComponent.h"
#include "components/GraphicsComponent.h"
#include "components/RenderingComponent.h"
#include "components/TransformComponent.h"
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
                               const GameSettings& settings,
                               GraphicsRegistry& graphicsRegistry,
                               std::shared_ptr<Renderer> renderer)
    : SubSystem(stopToken), m_settings(settings), m_graphicsRegistry(graphicsRegistry),
      m_renderer(std::move(renderer))
{
}

void game::ResourceLoader::loadEntities()
{
    spdlog::info("Loading entities...");

    auto& gameState = GameState::getInstance();

    auto size = m_settings.getWorldSizeInTiles();
    gameState.initializeStaticEntityMap(size.width, size.height);
    gameState.generateMap();
    // gameState.generateDebugMap();

    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            auto tile = gameState.createEntity();
            gameState.addComponent(tile, TransformComponent(i * 256, j * 256));
            gameState.addComponent(tile, RenderingComponent());
            gameState.addComponent(tile, EntityInfoComponent(2, rand() % 50 + 1));
            gameState.addComponent(tile, DirtyComponent());

            auto map = gameState.staticEntityMap;
            map.entityMap[i][j] = tile;

            GraphicsComponent gc;
            gc.entityID = tile;
            gc.entityType = 2; // Tile
            gameState.addComponent(tile, gc);
        }
    }

    {
        auto villager = gameState.createEntity();
        auto transform = TransformComponent(20 * 256, 20 * 256);
        transform.face(Direction::SOUTH);
        gameState.addComponent(villager, transform);
        gameState.addComponent(villager, RenderingComponent());
        gameState.addComponent(villager, EntityInfoComponent(3));
        gameState.addComponent(villager, ActionComponent(0));
        gameState.addComponent(villager, AnimationComponent());
        gameState.addComponent(villager, DirtyComponent());

        // villager goes idle by default
        CompUnit unit;
        unit.commandQueue.push(ObjectPool<CmdIdle>::acquire());
        gameState.addComponent(villager, unit);

        GraphicsComponent gc;
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
                auto transform = TransformComponent(i * 256 + 128, j * 256 + 128);
                transform.face(Direction::NORTHWEST);
                gameState.addComponent(tree, transform);
                gameState.addComponent(tree, RenderingComponent());
                GraphicsComponent gc;
                gc.debugOverlays.push_back({DebugOverlay::Type::CIRCLE, DebugOverlay::Color::RED,
                                            DebugOverlay::Anchor::BOTTOM_CENTER});
                gc.isStatic = true;
                gc.entityID = tree;
                gc.entityType = 4; // tree
                gameState.addComponent(tree, gc);
                gameState.addComponent(tree, EntityInfoComponent(4, rand() % 10));
                gameState.addComponent(tree, DirtyComponent());
            }
        }
    }

    spdlog::info("Entity loaded successfully.");
}

void ResourceLoader::init()
{
    loadEntities();
}
