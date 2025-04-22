#include "Renderer.h"

#include "FPSCounter.h"
#include "GameState.h"
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
#include <iostream>
#include <thread>

using namespace aion;
using namespace utils;

Renderer::Renderer(std::stop_source* stopSource,
                   const GameSettings& settings,
                   aion::GraphicsRegistry& graphicsRegistry,
                   Viewport& viewport,
                   EventLoop& eventLoop)
    : SubSystem(stopSource), settings(settings), graphicsRegistry(graphicsRegistry),
      viewport(viewport), eventLoop(eventLoop)
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
                               settings.getWindowDimensions().height, SDL_WINDOW_OPENGL);
    if (!window_)
    {
        spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    // SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d");

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
        // std::cout << "Rendering loop..." << counter++ << std::endl;
        fpsCounter.frame();
        running = handleEvents();

        eventLoop.handleEvents();

        // Event loop might have requested a viewport movement, apply the requested change
        // viewport.syncPosition();

        // handleGraphicInstructions();

        renderBackground();
        renderGameEntities();
        renderDebugInfo(fpsCounter);

        SDL_RenderPresent(renderer_);
        // SDL_Delay(16); // ~60 FPS
    }

    spdlog::info("Shutting down renderer...");

    SDL_DestroyWindow(window_);
    SDL_Quit();
    stopSource_->request_stop();
}

bool aion::Renderer::handleEvents()
{

    SDL_Event event;
    bool running = true;
    // auto message = ObjectPool<ThreadMessage>::acquire(ThreadMessage::Type::INPUT);
    bool alreadyHandled[static_cast<int>(Event::Type::MAX_TYPE_MARKER)] = {false};

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            running = false;
            break;
        }
        // else
        // {
        //     // spdlog::debug("Sending event: {}", event.type);

        //     // auto eventPtr = ObjectPool<SDL_Event>::acquire();
        //     // *eventPtr = event;
        //     // message->commandBuffer.push_back(static_cast<void*>(eventPtr));
        //     if (event.type == SDL_EVENT_KEY_DOWN)
        //     {
        //         // eventLoop->submitEvents(Event(Event::Type::KEY_DOWN, event));
        //         alreadyHandled[static_cast<int>(Event::Type::KEY_DOWN)] = true;

        //         if (event.key.key == SDLK_A)
        //         {
        //             spdlog::debug("Got key event: A ");

        //             viewport.setViewportPositionInPixels(
        //                 viewport.getViewportPositionInPixels() -
        //                 Vec2d(settings.getViewportMovingSpeed(), 0));
        //             // viewport.requestPositionChange(Vec2d(-settings.getViewportMovingSpeed(),
        //             0));
        //         }
        //         else if (event.key.key == SDLK_D)
        //         {
        //             viewport.setViewportPositionInPixels(
        //                 viewport.getViewportPositionInPixels() +
        //                 Vec2d(settings.getViewportMovingSpeed(), 0));
        //             // viewport.requestPositionChange(Vec2d(settings.getViewportMovingSpeed(),
        //             0));
        //         }
        //         else if (event.key.key == SDLK_W)
        //         {
        //             viewport.setViewportPositionInPixels(
        //                 viewport.getViewportPositionInPixels() - Vec2d(0,
        //                 settings.getViewportMovingSpeed()));
        //             // viewport.requestPositionChange(Vec2d(0,
        //             -settings.getViewportMovingSpeed()));
        //         }
        //         else if (event.key.key == SDLK_S)
        //         {
        //             spdlog::debug("Got key event: S ");

        //             viewport.setViewportPositionInPixels(
        //                 viewport.getViewportPositionInPixels() + Vec2d(0,
        //                 settings.getViewportMovingSpeed()));
        //             // viewport.requestPositionChange(Vec2d(0,
        //             settings.getViewportMovingSpeed()));
        //         }

        //     }
        //     //else if (event.type == SDL_EVENT_KEY_UP &&
        //     //         !alreadyHandled[static_cast<int>(Event::Type::KEY_UP)])
        //     //{
        //     //    // eventLoop->submitEvents(Event(Event::Type::KEY_UP, event));
        //     //    alreadyHandled[static_cast<int>(Event::Type::KEY_UP)] = true;
        //     //}
        //     //else if (event.type == SDL_EVENT_MOUSE_MOTION &&
        //     //         !alreadyHandled[static_cast<int>(Event::Type::MOUSE_MOVE)])
        //     //{
        //     //    // eventLoop->submitEvents(Event(Event::Type::MOUSE_MOVE, event));
        //     //    alreadyHandled[static_cast<int>(Event::Type::MOUSE_MOVE)] = true;
        //     //}
        //     //else if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
        //     //         !alreadyHandled[static_cast<int>(Event::Type::MOUSE_BTN_DOWN)])
        //     //{
        //     //    // eventLoop->submitEvents(Event(Event::Type::MOUSE_BTN_DOWN, event));
        //     //    alreadyHandled[static_cast<int>(Event::Type::MOUSE_BTN_DOWN)] = true;
        //     //}
        //     //else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP &&
        //     //         !alreadyHandled[static_cast<int>(Event::Type::MOUSE_BTN_UP)])
        //     //{
        //     //    // eventLoop->submitEvents(Event(Event::Type::MOUSE_BTN_UP, event));
        //     //    alreadyHandled[static_cast<int>(Event::Type::MOUSE_BTN_UP)] = true;
        //     //}

        // }
    }
    // TODO: Can optimize this to avoid getting a object from the pool at all
    // if (message->commandBuffer.empty())
    //     ObjectPool<ThreadMessage>::release(message);
    // else
    //     eventLoopQueue_.enqueue(message);
    return running;
}

// void aion::Renderer::handleGraphicInstructions()
// {
//     // spdlog::debug("Handling graphic instructions...");
//     maxQueueSize_ = std::max(maxQueueSize_, queueSize_);
//     queueSize_ = 0;

//     ThreadMessage* message = nullptr;

//     while (simulatorQueue.try_dequeue(message))
//     {
//         spdlog::debug("Handling graphic instructions...");

//         if (message->type != ThreadMessage::Type::RENDER)
//         {
//             spdlog::error("Invalid message type for Renderer: {}",
//             static_cast<int>(message->type)); throw std::runtime_error("Invalid message type for
//             Renderer.");
//         }

//         queueSize_++;
//         for (const auto& cmdPtr : message->commandBuffer)
//         {
//             auto instruction = static_cast<GraphicInstruction*>(cmdPtr);

//             if (instruction->type == GraphicInstruction::Type::ADD)
//             {
//                 auto& entry = graphicsRegistry.getGraphic(instruction->graphicsID);
//                 if (entry.image != nullptr)
//                 {
//                     auto& gc =
//                     aion::GameState::getInstance().getComponent<aion::GraphicsComponent>(
//                         instruction->entity);
//                     gc.graphicsID = instruction->graphicsID;
//                     gc.worldPosition = instruction->worldPosition;
//                     gc.texture = entry.image;
//                 }
//                 else
//                 {
//                     spdlog::error("Texture not found for entity: {}",
//                                   instruction->graphicsID.toString());
//                 }
//                 ObjectPool<GraphicInstruction>::release(instruction);
//             }
//         }
//     }
//     ObjectPool<ThreadMessage>::release(message);
// }

void aion::Renderer::renderDebugInfo(FPSCounter& counter)
{
    // spdlog::debug("Rendering debug info...");

    addDebugText("Average FPS: " + std::to_string(counter.getAverageFPS()));
    addDebugText("Queue depth: " + std::to_string(queueSize_));
    addDebugText("Max Queue depth: " + std::to_string(maxQueueSize_));
    addDebugText("Viewport: " + viewport.getViewportPositionInPixels().toString());
    addDebugText("Anchor Tile: " + anchorTilePixelsPos.toString());

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

    aion::GameState::getInstance()
        .getEntities<aion::TransformComponent, aion::EntityInfoComponent, aion::GraphicsComponent>()
        .each(
            [this](entt::entity entityID, aion::TransformComponent& transform,
                   aion::EntityInfoComponent& entityInfo, aion::GraphicsComponent& gc)
            {
                GraphicsID graphicsID(entityInfo.entityType, 0,
                                      0, // TODO: FIX this
                                      transform.getDirection());

                auto& entry = graphicsRegistry.getGraphic(graphicsID);
                if (entry.image != nullptr)
                {
                    gc.graphicsID = graphicsID;
                    gc.worldPosition = transform.position;
                    gc.texture = entry.image;
                }
                else
                {
                    spdlog::error("Texture not found for entity: {}", graphicsID.toString());
                }
            });

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

void aion::Renderer::renderBackground()
{
    // spdlog::debug("Rendering background...");

    SDL_SetRenderDrawColor(renderer_, 30, 30, 30, 255);
    SDL_RenderClear(renderer_);
}
