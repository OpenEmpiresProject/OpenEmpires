#include "Renderer.h"

#include "FPSCounter.h"
#include "GameState.h"
#include "Logger.h"
#include "Renderer.h"
#include "components/ActionComponent.h"
#include "components/EntityInfoComponent.h"
#include "components/GraphicsComponent.h"
#include "components/TransformComponent.h"

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h> // Ensure SDL_image is included
#include <iostream>
#include <thread>

using namespace aion;

Renderer::Renderer(std::stop_source *stopSource, const GameSettings &settings,
                   aion::GraphicsRegistry &graphicsRegistry, ThreadQueue &simulatorQueue)
    : SubSystem(stopSource), settings(settings), graphicsRegistry(graphicsRegistry),
      threadQueue_(simulatorQueue)
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
    FPSCounter fpsCounter;

    while (running)
    {
        fpsCounter.frame();
        running = handleEvents();

        handleGraphicInstructions();

        renderBackground();
        renderGameEntities();
        renderDebugInfo(fpsCounter);

        SDL_RenderPresent(renderer_);
        // SDL_Delay(16); // ~60 FPS
    }

    SDL_DestroyWindow(window_);
    SDL_Quit();
    stopSource_->request_stop();

    spdlog::shutdown();
    spdlog::drop_all();
}

bool aion::Renderer::handleEvents()
{
    SDL_Event event;
    bool running = true;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            running = false;
            break;
        }
    }
    return running;
}

void aion::Renderer::handleGraphicInstructions()
{
    // spdlog::debug("Handling graphic instructions...");
    maxQueueSize_ = std::max(maxQueueSize_, queueSize_);
    queueSize_ = 0;

    std::vector<GraphicInstruction> instructionList;

    while (threadQueue_.try_dequeue(instructionList))
    {
        queueSize_++;
        for (const auto &instruction : instructionList)
        {
            if (instruction.type == GraphicInstruction::Type::ADD)
            {
                auto &entry = graphicsRegistry.getGraphic(instruction.graphicsID);
                if (entry.image != nullptr)
                {
                    auto &gc = aion::GameState::getInstance().getComponent<aion::GraphicsComponent>(
                        instruction.entity);
                    gc.graphicsID = instruction.graphicsID;
                    gc.worldPosition = instruction.worldPosition;
                    gc.texture = entry.image;
                }
                else
                {
                    spdlog::error("Texture not found for entity: {}",
                                  instruction.graphicsID.toString());
                }
            }
        }
    }
}

void aion::Renderer::renderDebugInfo(FPSCounter &counter)
{
    // spdlog::debug("Rendering debug info...");

    addDebugText("Average FPS: " + std::to_string(counter.getAverageFPS()));
    addDebugText("Queue depth: " + std::to_string(queueSize_));
    addDebugText("Max Queue depth: " + std::to_string(maxQueueSize_));

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, SDL_ALPHA_OPAQUE); /* white, full alpha */

    int y = 10;
    for (const auto &line : debugTexts)
    {
        SDL_RenderDebugText(renderer_, 10, y, line.c_str());
        y += 20;
    }

    clearDebugTexts();
}

void aion::Renderer::renderGameEntities()
{
    // spdlog::debug("Rendering game entities...");

    aion::GameState::getInstance().getEntities<aion::GraphicsComponent>().each(
        [this](aion::GraphicsComponent &gc)
        {
            SDL_FRect dstRect = {gc.worldPosition.x, gc.worldPosition.y, 100, 100};
            SDL_RenderTexture(renderer_, gc.texture, nullptr, &dstRect);
        });
}

void aion::Renderer::renderBackground()
{
    // spdlog::debug("Rendering background...");

    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
    SDL_RenderClear(renderer_);
}
