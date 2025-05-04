#include "ResourceLoader.h"

#include "GameState.h"
#include "Logger.h"
#include "Renderer.h"
#include "SubSystemRegistry.h"
#include "components/ActionComponent.h"
#include "components/AnimationComponent.h"
#include "components/DirtyComponent.h"
#include "components/EntityInfoComponent.h"
#include "components/GraphicsComponent.h"
#include "components/TransformComponent.h"

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

    auto size = _settings.getWorldSizeInTiles();
    GameState::getInstance().initializeStaticEntityMap(size.width, size.height);
    GameState::getInstance().generateMap();
    // GameState::getInstance().generateDebugMap();

    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            auto tile = GameState::getInstance().createEntity();
            GameState::getInstance().addComponent(tile, TransformComponent(i * 256, j * 256));
            GameState::getInstance().addComponent(tile, GraphicsComponent());
            GameState::getInstance().addComponent(tile, EntityInfoComponent(2, rand() % 50 + 1));
            auto map = GameState::getInstance().staticEntityMap;
            map.entityMap[i][j] = tile;
        }
    }

    {
        auto villager = GameState::getInstance().createEntity();
        auto transform = TransformComponent(20 * 256, 20 * 256);
        transform.face(utils::Direction::SOUTH);
        GameState::getInstance().addComponent(villager, transform);
        GameState::getInstance().addComponent(villager, GraphicsComponent());
        GameState::getInstance().addComponent(villager, EntityInfoComponent(3));
        GameState::getInstance().addComponent(villager, ActionComponent(0));
        GameState::getInstance().addComponent(villager, AnimationComponent());
        GameState::getInstance().addComponent(villager, DirtyComponent());
    }

    // for staticEntityMap in GameState, create entities for each tree marked with 1 in the double
    // array
    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            if (GameState::getInstance().staticEntityMap.map[i][j] == 1)
            {
                auto tree = GameState::getInstance().createEntity();
                auto transform = TransformComponent(i * 256 + 128, j * 256 + 128);
                GameState::getInstance().addComponent(tree, transform);
                GameState::getInstance().addComponent(tree, GraphicsComponent());
                GameState::getInstance().addComponent(tree,
                                                      EntityInfoComponent(4, rand() % 10, true));
                GameState::getInstance().addComponent(tree, DirtyComponent());
            }
        }
    }

    spdlog::info("Entity loaded successfully.");
}

void ResourceLoader::init()
{
    loadEntities();
}
