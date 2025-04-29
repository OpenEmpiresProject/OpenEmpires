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
    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            auto entity = GameState::getInstance().createEntity();
            GameState::getInstance().addComponent(entity, TransformComponent(i * 256, j * 256));
            GameState::getInstance().addComponent(entity, GraphicsComponent());
            GameState::getInstance().addComponent(entity, EntityInfoComponent(2));
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

    {
        auto tree = GameState::getInstance().createEntity();
        auto transform = TransformComponent(35 * 256, 35 * 256);
        transform.face(utils::Direction::SOUTH);
        GameState::getInstance().addComponent(tree, transform);
        GameState::getInstance().addComponent(tree, GraphicsComponent());
        GameState::getInstance().addComponent(tree, EntityInfoComponent(4));
        GameState::getInstance().addComponent(tree, DirtyComponent());
    }

    GameState::getInstance().initializeStaticEntityMap(size.width, size.height);
    GameState::getInstance().generateMap();
    
    // for staticEntityMap in GameState, create entities for each tree marked with 1 in the double array
    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            if (GameState::getInstance().staticEntityMap.map[i][j] == 1)
            {
                auto entity = GameState::getInstance().createEntity();
                auto transform = TransformComponent(i * 256, j * 256);
                GameState::getInstance().addComponent(entity, transform);
                GameState::getInstance().addComponent(entity, GraphicsComponent());
                GameState::getInstance().addComponent(entity, EntityInfoComponent(4));
                GameState::getInstance().addComponent(entity, DirtyComponent());
            }
        }
    }

    spdlog::info("Entity loaded successfully.");
}

void ResourceLoader::init()
{
    loadEntities();
}
