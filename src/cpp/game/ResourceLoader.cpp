#include "ResourceLoader.h"
// #include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h> // Ensure SDL_image is included
#include <SDL3/SDL.h>
#include "SubSystemRegistry.h"
#include "Renderer.h"
#include "GameState.h"
#include "components/TransformComponent.h"
#include "components/GraphicsComponent.h"
#include "components/ActionComponent.h"

using namespace game;
using namespace aion;

ResourceLoader::ResourceLoader(
    const aion::GameSettings &settings, 
    aion::GraphicsRegistry &graphicsRegistry,
    aion::Renderer &renderer)
    : _settings(settings), graphicsRegistry(graphicsRegistry), renderer(renderer)
{
}

void ResourceLoader::loadTextures()
{
    std::cout << "Loading textures..." << std::endl;
    SDL_Surface* surface = SDL_LoadBMP("E:/Projects/openEmpires/OpenEmpires/test.bmp");
    if (!surface) {
        std::cerr << "Failed to load the bitmap: " << SDL_GetError() << "\n";
        throw std::runtime_error(std::string("Failed to load the bitmap") + SDL_GetError());
    }
    std::cout << "Loaded bitmap successfully" << std::endl;
    SDL_Renderer* sdlRenderer = renderer.getSDLRenderer();
    std::cout << "Renderer: " << sdlRenderer << std::endl;
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(sdlRenderer, surface);
    if (texture == nullptr) {
        std::cerr << "Failed to create texture from surface: " << SDL_GetError() << "\n";
        throw std::runtime_error(std::string("Failed to create texture from surface") + SDL_GetError());
    }   
    SDL_DestroySurface(surface);
    // Store the texture in the graphics registry
    aion::GraphicsID id;
    id.entitytType = 1; // Example entity type
    id.actionType = 1; // Example action type
    id.frame = 0; // Example frame
    id.direction = utils::Direction::NORTH; // Example direction
    aion::GraphicsEntry entry{texture, {0, 0}}; // Example anchor position
    graphicsRegistry.registerGraphic(id, entry);
}

void game::ResourceLoader::loadEntities()
{
    aion::GraphicsID id;
    id.entitytType = 1; // Example entity type
    id.actionType = 1; // Example action type
    id.frame = 0; // Example frame
    id.direction = utils::Direction::NORTH; // Example direction

    auto testEntity = GameState::getInstance().createEntity();
    GameState::getInstance().addComponent(testEntity, TransformComponent(100, 100));
    GameState::getInstance().addComponent(testEntity, GraphicsComponent(id, 0));
    GameState::getInstance().addComponent(testEntity, ActionComponent(0));
}

void ResourceLoader::init()
{
    loadEntities();
    loadTextures();
}
