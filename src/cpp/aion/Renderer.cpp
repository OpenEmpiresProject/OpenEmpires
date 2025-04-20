#include "Renderer.h"

#include "GameState.h"
#include "Logger.h"
#include "components/GraphicsComponent.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h> // Ensure SDL_image is included
#include <iostream>
#include <thread>

using namespace aion;

Renderer::Renderer(const GameSettings &settings, aion::GraphicsRegistry &graphicsRegistry)
    : settings(settings), graphicsRegistry(graphicsRegistry)
{
    running_ = false;
}

Renderer::~Renderer()
{
    cleanup();
    // IMG_Quit();
    SDL_Quit();
}

void Renderer::init()
{
    if (!running_)
    {
        running_ = true;
        renderThread_ = std::thread(&Renderer::threadEntry, this);
    }
}

void Renderer::shutdown()
{
    running_ = false;
    if (renderThread_.joinable())
    {
        // renderThread_.request_stop();
        renderThread_.join();
    }
}

void Renderer::cleanup()
{
    // if (texture_) SDL_DestroyTexture(texture_);
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    if (window_)
        SDL_DestroyWindow(window_);
}

void Renderer::threadEntry()
{
    initSDL();
    renderingLoop();
}

void Renderer::initSDL()
{
    spdlog::info("Initializing SDL...");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        spdlog::error("SDL_Init failed: {}", SDL_GetError());
        throw std::runtime_error("SDL_Init failed");
    }

    window_ = SDL_CreateWindow(settings.getTitle().c_str(), settings.getWindowDimensions().width,
                               settings.getWindowDimensions().height, 0);
    if (!window_)
    {
        spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    renderer_ = SDL_CreateRenderer(window_, nullptr);
    if (!renderer_)
    {
        spdlog::error("SDL_CreateRenderer failed: {}", SDL_GetError());
        throw std::runtime_error("SDL_CreateRenderer failed");
    }
    {
        std::lock_guard<std::mutex> lock(sdlInitMutex_);
        sdlInitialized_ = true;
    }
    sdlInitCV_.notify_all();
    spdlog::info("SDL initialized successfully");
}

void aion::Renderer::renderingLoop()
{
    spdlog::info("Starting rendering loop...");
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

        aion::GameState::getInstance().getEntities<aion::GraphicsComponent>().each(
            [this](entt::entity entity, aion::GraphicsComponent &graphics)
            {
                auto &graphicID = graphics.graphicID;
                auto &entry = graphicsRegistry.getGraphic(graphicID);
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