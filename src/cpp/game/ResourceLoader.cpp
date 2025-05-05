#include "ResourceLoader.h"

#include "GameState.h"
#include "Logger.h"
#include "Renderer.h"
#include "SubSystemRegistry.h"
#include "components/ActionComponent.h"
#include "components/AnimationComponent.h"
#include "components/DirtyComponent.h"
#include "components/EntityInfoComponent.h"
#include "components/RenderingComponent.h"
#include "components/TransformComponent.h"
#include "components/GraphicsComponent.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <filesystem>

namespace fs = std::filesystem;
using namespace game;
using namespace aion;

ResourceLoader::ResourceLoader(std::stop_token* stopToken,
                               const aion::GameSettings& settings,
                               aion::GraphicsRegistry& graphicsRegistry,
                               aion::Renderer& renderer)
    : SubSystem(stopToken), _settings(settings), graphicsRegistry(graphicsRegistry),
      renderer(renderer)
{
}

void game::ResourceLoader::loadEntities()
{
    spdlog::info("Loading entities...");

    auto& gameState = GameState::getInstance();

    auto size = _settings.getWorldSizeInTiles();
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

    // {
    //     auto villager = gameState.createEntity();
    //     auto transform = TransformComponent(20 * 256, 20 * 256);
    //     transform.face(utils::Direction::SOUTH);
    //     gameState.addComponent(villager, transform);
    //     gameState.addComponent(villager, RenderingComponent());
    //     gameState.addComponent(villager, EntityInfoComponent(3));
    //     gameState.addComponent(villager, ActionComponent(0));
    //     gameState.addComponent(villager, AnimationComponent());
    //     gameState.addComponent(villager, DirtyComponent());

    //     GraphicsComponent gc;
    //     gc.entityID = villager;
    //     gc.entityType = 3; // Villager
    //     gameState.addComponent(villager, gc);
    // }

    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            if (gameState.staticEntityMap.map[i][j] == 1)
            {
                auto tree = gameState.createEntity();
                auto transform = TransformComponent(i * 256 + 128, j * 256 + 128);
                transform.face(utils::Direction::NORTHWEST);
                gameState.addComponent(tree, transform);
                gameState.addComponent(tree, RenderingComponent());
                GraphicsComponent gc;
                gc.setDebugHighlightType(utils::DebugHighlightType::TILE_TREE_MARK);
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
