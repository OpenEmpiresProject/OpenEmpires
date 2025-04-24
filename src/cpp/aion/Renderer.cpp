#include "Renderer.h"

#include "FPSCounter.h"
#include "GameState.h"
#include "GraphicsLoader.h"
#include "Logger.h"
#include "ObjectPool.h"
#include "Renderer.h"
#include "components/ActionComponent.h"
#include "components/EntityInfoComponent.h"
#include "components/GraphicsComponent.h"
#include "components/TransformComponent.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h> // Ensure SDL_image is included
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <thread>
namespace fs = std::filesystem;
using namespace aion;
using namespace utils;
using namespace std::chrono;

Renderer::Renderer(std::stop_source* stopSource,
                   const GameSettings& settings,
                   aion::GraphicsRegistry& graphicsRegistry,
                   ThreadQueue& simulatorQueue,
                   Viewport& viewport)
    : SubSystem(stopSource), settings(settings), graphicsRegistry(graphicsRegistry),
      simulatorQueue(simulatorQueue), viewport(viewport)
{
    running_ = false;
    lastTickTime = steady_clock::now();
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
    if (renderer_)
        SDL_DestroyRenderer(renderer_);
    if (window_)
        SDL_DestroyWindow(window_);
}

void Renderer::threadEntry()
{
    initSDL();
    GraphicsLoader(renderer_, graphicsRegistry).loadAllGraphics();
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
                               settings.getWindowDimensions().height, SDL_WINDOW_OPENGL);
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
    int counter = 0;

    while (running)
    {
        auto start = SDL_GetTicks();
        fpsCounter.frame();
        running = handleEvents();
        generateTicks();

        handleGraphicInstructions();
        renderBackground();
        renderGameEntities();
        renderDebugInfo(fpsCounter);

        SDL_RenderPresent(renderer_);

        int delay = (1000 / settings.getTargetFPS()) - (SDL_GetTicks() - start);
        delay = std::max(1, delay);

        SDL_Delay(delay);

        fpsCounter.sleptFor(delay);
        fpsCounter.getTotalSleep();
    }

    spdlog::info("Shutting down renderer...");

    SDL_DestroyWindow(window_);
    SDL_Quit();
    stopSource_->request_stop();
}

bool aion::Renderer::handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            return false;
        }
    }
    return true;
}

void aion::Renderer::handleGraphicInstructions()
{
    // spdlog::debug("Handling graphic instructions...");
    maxQueueSize_ = std::max(maxQueueSize_, queueSize_);
    queueSize_ = 0;

    ThreadMessage* message = nullptr;

    while (simulatorQueue.try_dequeue(message))
    {
        spdlog::debug("Handling graphic instructions...");

        if (message->type != ThreadMessage::Type::RENDER)
        {
            spdlog::error("Invalid message type for Renderer: {}", static_cast<int>(message->type));
            throw std::runtime_error("Invalid message type for Renderer.");
        }

        queueSize_++;
        for (const auto& cmdPtr : message->commandBuffer)
        {
            auto instruction = static_cast<GraphicInstruction*>(cmdPtr);

            if (instruction->type == GraphicInstruction::Type::ADD)
            {
                auto& entry = graphicsRegistry.getGraphic(instruction->graphicsID);
                if (entry.image != nullptr)
                {
                    auto& gc = aion::GameState::getInstance().getComponent<aion::GraphicsComponent>(
                        instruction->entity);
                    gc.graphicsID = instruction->graphicsID;
                    gc.worldPosition = instruction->worldPosition;
                    gc.texture = entry.image;
                }
                else
                {
                    spdlog::error("Texture not found for entity: {}",
                                  instruction->graphicsID.toString());
                }
                ObjectPool<GraphicInstruction>::release(instruction);
            }
        }
    }
    ObjectPool<ThreadMessage>::release(message);
}

void aion::Renderer::renderDebugInfo(FPSCounter& counter)
{
    // spdlog::debug("Rendering debug info...");

    addDebugText("Average FPS    : " + std::to_string(counter.getAverageFPS()));
    addDebugText("Avg Sleep/frame: " + std::to_string(counter.getAverageSleepMs()));
    addDebugText("Queue depth    : " + std::to_string(queueSize_));
    addDebugText("Max Queue depth: " + std::to_string(maxQueueSize_));
    addDebugText("Viewport       : " + viewport.getViewportPositionInPixels().toString());
    addDebugText("Anchor Tile    : " + anchorTilePixelsPos.toString());

    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, SDL_ALPHA_OPAQUE); /* white, full alpha */

    int y = 10;
    for (const auto& line : debugTexts)
    {
        SDL_RenderDebugText(renderer_, 10, y, line.c_str());
        y += 20;
    }

    clearDebugTexts();
}

void aion::Renderer::renderGameEntities()
{
    // spdlog::debug("Rendering game entities...");

    bool isFirst = true;
    SDL_FRect dstRect = {0, 0, 97, 49};

    aion::GameState::getInstance().getEntities<aion::GraphicsComponent>().each(
        [this, &isFirst, &dstRect](aion::GraphicsComponent& gc)
        {
            auto screenPos = viewport.feetToScreenUnits(gc.worldPosition);

            if (isFirst)
            {
                anchorTilePixelsPos = viewport.feetToPixels(gc.worldPosition);
                isFirst = false;
            }

            dstRect.x = screenPos.x;
            dstRect.y = screenPos.y;
            SDL_RenderTexture(renderer_, gc.texture, nullptr, &dstRect);
        });
}

void Renderer::renderBackground()
{
    // spdlog::debug("Rendering background...");

    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
    SDL_RenderClear(renderer_);
}

void Renderer::generateTicks()
{
    const auto tickDelay = milliseconds(1000 / settings.getTicksPerSecond());
    auto now = steady_clock::now();
    if (now - lastTickTime >= tickDelay)
    {
        lastTickTime = now;
        onTick();
    }
}

void Renderer::onTick()
{
    handleViewportMovement();
}

void aion::Renderer::handleViewportMovement()
{
    auto keyStates = SDL_GetKeyboardState(nullptr);
    if (keyStates[SDL_SCANCODE_W])
    {
        viewport.setViewportPositionInPixelsWithBounryChecking(viewport.getViewportPositionInPixels() -
                                             Vec2d(0, settings.getViewportMovingSpeed()));
    }
    if (keyStates[SDL_SCANCODE_A])
    {
        viewport.setViewportPositionInPixelsWithBounryChecking(viewport.getViewportPositionInPixels() -
                                             Vec2d(settings.getViewportMovingSpeed(), 0));
    }
    if (keyStates[SDL_SCANCODE_S])
    {
        viewport.setViewportPositionInPixelsWithBounryChecking(viewport.getViewportPositionInPixels() +
                                             Vec2d(0, settings.getViewportMovingSpeed()));
    }
    if (keyStates[SDL_SCANCODE_D])
    {
        viewport.setViewportPositionInPixelsWithBounryChecking(viewport.getViewportPositionInPixels() +
                                             Vec2d(settings.getViewportMovingSpeed(), 0));
    }
}
