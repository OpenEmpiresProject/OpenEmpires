#include "Renderer.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h> // Ensure SDL_image is included
#include <iostream>
#include <thread>
#include "GameState.h"
#include "components/GraphicsComponent.h"
// #include <SDL3/SDL_main.h>

using namespace aion;

Renderer::Renderer(const GameSettings& settings, aion::GraphicsRegistry& graphicsRegistry)
    : settings(settings), graphicsRegistry(graphicsRegistry)
{
    running_ = false;

    // if (IMG_Init(IMG_INIT_PNG) == 0) {
    //     throw std::runtime_error("IMG_Init failed");
    // }   
}

Renderer::~Renderer() {
    cleanup();
    // IMG_Quit();
    SDL_Quit();
}

void Renderer::init()
{
    std::cout<< "renderer init" << std::endl;

    if (!running_) {
        running_ = true;
        renderThread_ = std::thread(&Renderer::threadEntry, this);
    }
}

void Renderer::shutdown()
{
    running_ = false;
    if (renderThread_.joinable()) {
        // renderThread_.request_stop();
        renderThread_.join();
    }
}

void Renderer::cleanup() {
    // if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_) SDL_DestroyRenderer(renderer_);
    if (window_) SDL_DestroyWindow(window_);
}

// bool Renderer::loadTexture(const std::string& imagePath) {
//     SDL_Surface* surface = IMG_Load(imagePath.c_str());
//     if (!surface) {
//         // std::cerr << "Failed to load image: " << IMG_GetError() << "\n";
//         return false;
//     }

//     texture_ = SDL_CreateTextureFromSurface(renderer_, surface);
//     SDL_DestroySurface(surface);

//     return texture_ != nullptr;
// }

void Renderer::threadEntry() {
    std::cout<< "thead entry" << std::endl;
    initSDL();
    renderingLoop();
}

void Renderer::initSDL()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error("SDL_Init failed");
    }

    window_ = SDL_CreateWindow(
        settings.getTitle().c_str(), 
        settings.getWindowDimensions().width, 
        settings.getWindowDimensions().height, 
        0);
    if (!window_)
    {
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_)
    {
        throw std::runtime_error("SDL_CreateRenderer failed");
    }
    std::cout<< "SDL initialized" << std::endl;
    {
        std::lock_guard<std::mutex> lock(sdlInitMutex_);
        sdlInitialized_ = true;
    }
    sdlInitCV_.notify_all();
    std::cout<< "Notified that SDL is initialized" << std::endl;
}

void aion::Renderer::renderingLoop()
{
    bool running = true;
    // while (!stopToken.stop_requested()) {
    while (running)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                // stop();
                running = false;
                break;
            }
        }

        SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
        SDL_RenderClear(renderer_);

        aion::GameState::getInstance().getEntities<aion::GraphicsComponent>().each([this](entt::entity entity, aion::GraphicsComponent& graphics) {
            auto& graphicID = graphics.graphicID;
            auto& entry = graphicsRegistry.getGraphic(graphicID);
            if (entry.image != nullptr)
            {
                SDL_FRect dstRect = {100, 100, 100, 100}; // Example destination rect
                SDL_RenderTexture(renderer_, entry.image, nullptr, &dstRect);
            }
        });

        // if (texture_)
        // {
        //     SDL_FRect dstRect = {100, 100, 100, 100}; // Example draw position
        //     SDL_RenderTexture(renderer_, texture_, nullptr, &dstRect);
        // }

        SDL_RenderPresent(renderer_);
        SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyWindow(window_);
    SDL_Quit();
}