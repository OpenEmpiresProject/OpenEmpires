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
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>

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

void ResourceLoader::loadTextures()
{
    spdlog::info("Loading textures...");

    SDL_Surface* surface =
        SDL_LoadBMP("E:/Projects/openEmpires/OpenEmpires/asserts/images/terrain/tile_test.bmp");
    if (!surface)
    {
        spdlog::error("Failed to load the bitmap: {}", SDL_GetError());
        throw std::runtime_error(std::string("Failed to load the bitmap") + SDL_GetError());
    }
    // SDL_DisplayFormat(surface);
    auto formatDetails = SDL_GetPixelFormatDetails(surface->format);
    auto pallete = SDL_GetSurfacePalette(surface);
    auto rgb = SDL_MapRGB(formatDetails, pallete, 0xFF, 0, 0xFF);
    bool success = SDL_SetSurfaceColorKey(surface, true, rgb);
    SDL_Renderer* sdlRenderer = renderer.getSDLRenderer();
    spdlog::debug("SDL_Renderer is obtained successfully.");

    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
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

    aion::GraphicsID id2;
    id2.entitytType = 2; // Tile
    id2.actionType = 0;
    id2.frame = 0;
    id2.direction = utils::Direction::NORTH;
    aion::GraphicsEntry entry2{texture, {0, 0}};
    graphicsRegistry.registerGraphic(id2, entry2);

    spdlog::info("Texture loaded and registered successfully.");
}

void game::ResourceLoader::loadEntities()
{
    spdlog::info("Loading entities...");
    // auto testEntity = GameState::getInstance().createEntity();
    // GameState::getInstance().addComponent(testEntity, TransformComponent(100, 100));
    // GameState::getInstance().addComponent(testEntity, GraphicsComponent());
    // GameState::getInstance().addComponent(testEntity, ActionComponent(1));
    // GameState::getInstance().addComponent(testEntity, EntityInfoComponent(1));

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

    spdlog::info("Entity loaded successfully.");
}

void ResourceLoader::init()
{
    loadEntities();
    loadTextures();
}
