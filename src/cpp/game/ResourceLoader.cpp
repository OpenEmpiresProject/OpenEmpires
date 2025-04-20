#include "ResourceLoader.h"

#include "GameState.h"
#include "Logger.h"
#include "Renderer.h"
#include "SubSystemRegistry.h"
#include "components/ActionComponent.h"
#include "components/EntityInfoComponent.h"
#include "components/GraphicsComponent.h"
#include "components/TransformComponent.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

using namespace game;
using namespace aion;

ResourceLoader::ResourceLoader(const aion::GameSettings &settings,
                               aion::GraphicsRegistry &graphicsRegistry, aion::Renderer &renderer)
    : _settings(settings), graphicsRegistry(graphicsRegistry), renderer(renderer)
{
}

void ResourceLoader::loadTextures()
{
    spdlog::info("Loading textures...");

    SDL_Surface *surface = SDL_LoadBMP("E:/Projects/openEmpires/OpenEmpires/test.bmp");
    if (!surface)
    {
        spdlog::error("Failed to load the bitmap: {}", SDL_GetError());
        throw std::runtime_error(std::string("Failed to load the bitmap") + SDL_GetError());
    }
    SDL_Renderer *sdlRenderer = renderer.getSDLRenderer();
    spdlog::debug("SDL_Renderer is obtained successfully.");

    SDL_Texture *texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    if (texture == nullptr)
    {
        spdlog::error("Failed to create texture from surface: {}", SDL_GetError());
        throw std::runtime_error(std::string("Failed to create texture from surface") +
                                 SDL_GetError());
    }
    SDL_DestroySurface(surface);
    // Store the texture in the graphics registry
    aion::GraphicsID id;
    id.entitytType = 1;                         // Example entity type
    id.actionType = 1;                          // Example action type
    id.frame = 0;                               // Example frame
    id.direction = utils::Direction::NORTH;     // Example direction
    aion::GraphicsEntry entry{texture, {0, 0}}; // Example anchor position
    graphicsRegistry.registerGraphic(id, entry);

    spdlog::info("Texture loaded and registered successfully.");
}

void game::ResourceLoader::loadEntities()
{
    spdlog::info("Loading entities...");
    auto testEntity = GameState::getInstance().createEntity();
    GameState::getInstance().addComponent(testEntity, TransformComponent(100, 100));
    GameState::getInstance().addComponent(testEntity, GraphicsComponent());
    GameState::getInstance().addComponent(testEntity, ActionComponent(1));
    GameState::getInstance().addComponent(testEntity, EntityInfoComponent(1));

    spdlog::info("Entity loaded successfully.");
}

void ResourceLoader::init()
{
    loadEntities();
    loadTextures();
}
