#include "Renderer.h"

#include "AtlasGeneratorBasic.h"
#include "Event.h"
#include "FPSCounter.h"
#include "GameState.h"
#include "GraphicsLoader.h"
#include "Renderer.h"
#include "SDL3_gfxPrimitives.h"
#include "ServiceRegistry.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompEntityInfo.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
#include <SDL3_image/SDL_image.h> // Ensure SDL_image is included
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <thread>
namespace fs = std::filesystem;
using namespace aion;
using namespace std::chrono;

Renderer::Renderer(std::stop_source* stopSource,
                   GraphicsRegistry& graphicsRegistry,
                   ThreadSynchronizer<FrameData>& synchronizer)
    : SubSystem(stopSource), m_settings(ServiceRegistry::getInstance().getService<GameSettings>()),
      m_graphicsRegistry(graphicsRegistry),
      m_coordinates(ServiceRegistry::getInstance().getService<GameSettings>()),
      m_zBucketsSize(
          ServiceRegistry::getInstance().getService<GameSettings>()->getWorldSizeInTiles().height *
          Constants::FEET_PER_TILE * 3),
      m_zBuckets(
          ServiceRegistry::getInstance().getService<GameSettings>()->getWorldSizeInTiles().height *
          Constants::FEET_PER_TILE * 3),
      m_synchronizer(synchronizer)
{
    m_running = false;
    m_lastTickTime = steady_clock::now();
    spdlog::info("Max z-order: {}", m_coordinates.getMaxZOrder());

    auto center = m_coordinates.getMapCenterInFeet();
    auto centerPixels = m_coordinates.feetToPixels(center);
    centerPixels -= Vec2d(m_settings->getWindowDimensions().width / 2,
                          m_settings->getWindowDimensions().height / 2);
    m_coordinates.setViewportPositionInPixels(centerPixels);
}

Renderer::~Renderer()
{
    cleanup();
    // IMG_Quit();
    SDL_Quit();
}

void Renderer::init()
{
    if (!m_running)
    {
        m_running = true;
        m_renderThread = std::thread(&Renderer::threadEntry, this);
    }
}

void Renderer::shutdown()
{
    m_running = false;
    if (m_renderThread.joinable())
    {
        // m_renderThread.request_stop();
        m_renderThread.join();
    }
}

void Renderer::cleanup()
{
    if (m_renderer)
        SDL_DestroyRenderer(m_renderer);
    if (m_window)
        SDL_DestroyWindow(m_window);
}

void Renderer::threadEntry()
{
    initSDL();
    AtlasGeneratorBasic atlasGenerator;
    GraphicsLoader(m_renderer, m_graphicsRegistry, atlasGenerator).loadAllGraphics();
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

    m_window =
        SDL_CreateWindow(m_settings->getTitle().c_str(), m_settings->getWindowDimensions().width,
                         m_settings->getWindowDimensions().height, SDL_WINDOW_OPENGL);
    if (!m_window)
    {
        spdlog::error("SDL_CreateWindow failed: {}", SDL_GetError());
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    m_renderer = SDL_CreateRenderer(m_window, nullptr);
    if (!m_renderer)
    {
        spdlog::error("SDL_CreateRenderer failed: {}", SDL_GetError());
        throw std::runtime_error("SDL_CreateRenderer failed");
    }
    {
        std::lock_guard<std::mutex> lock(m_sdlInitMutex);
        m_sdlInitialized = true;
    }
    m_sdlInitCV.notify_all();
    spdlog::info("SDL initialized successfully");
}

void Renderer::renderingLoop()
{
    spdlog::info("Starting rendering loop...");
    bool running = true;
    FPSCounter fpsCounter;
    int counter = 0;

    while (running)
    {
        auto frame = m_synchronizer.getReceiverFrameData().frameNumber;
        // spdlog::info("Rendering frame {}", frame);

        auto start = SDL_GetTicks();
        fpsCounter.frame();
        running = handleEvents();

        if (!running)
        {
            break;
        }
        generateTicks();

        updateRenderingComponents();
        renderBackground();
        renderGameEntities();
        renderSelectionBox();
        renderDebugInfo(fpsCounter);

        SDL_RenderPresent(m_renderer);

        int delay = (1000 / m_settings->getTargetFPS()) - (SDL_GetTicks() - start);
        delay = std::max(1, delay);

        SDL_Delay(delay);

        fpsCounter.sleptFor(delay);
        fpsCounter.getTotalSleep();

        m_synchronizer.waitForSender();
    }

    spdlog::info("Shutting down renderer...");

    SDL_DestroyWindow(m_window);
    SDL_Quit();
    m_synchronizer.shutdown();
    m_stopSource->request_stop();
}

bool Renderer::handleEvents()
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
        {
            return false;
        }
        // handle mouse click events
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
        {
            if (event.button.button == SDL_BUTTON_LEFT)
            {
                Vec2d mousePosScreenUnits(event.button.x, event.button.y);
                m_lastMouseClickPosInFeet = m_coordinates.screenUnitsToFeet(mousePosScreenUnits);
                m_lastMouseClickPosInTiles = m_coordinates.feetToTiles(m_lastMouseClickPosInFeet);

                Event clickEvent(
                    Event::Type::MOUSE_BTN_UP,
                    MouseClickData{MouseClickData::Button::LEFT, m_lastMouseClickPosInFeet});

                m_isSelecting = true;
                m_selectionStartPosScreenUnits = mousePosScreenUnits;
                m_selectionEndPosScreenUnits = mousePosScreenUnits;
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP)
        {
            m_isSelecting = false;
        }
        else if (event.type == SDL_EVENT_KEY_UP)
        {
            if (event.key.scancode == SDL_SCANCODE_1)
            {
                m_showStaticEntities = !m_showStaticEntities;
            }
            else if (event.key.scancode == SDL_SCANCODE_2)
            {
                m_showDebugInfo = !m_showDebugInfo;
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            if (m_isSelecting)
            {
                Vec2d mousePosScreenUnits(event.button.x, event.button.y);
                m_selectionEndPosScreenUnits = mousePosScreenUnits;
            }
        }
    }
    return true;
}

/**
 * @brief Updates the graphic components by processing messages from the simulator.
 *
 * This function handles graphic instructions by dequeuing messages from the simulator queue
 * and processing their command buffers. It updates the graphics components of entities
 * based on the instructions provided in the messages. If a texture is not found for an entity,
 * an error is logged. Invalid message types or other unexpected conditions will throw exceptions.
 *
 * @throws std::runtime_error If a message with an invalid type is encountered.
 */
void Renderer::updateRenderingComponents()
{
    // spdlog::debug("Handling graphic instructions...");
    auto& frameData = m_synchronizer.getReceiverFrameData().graphicUpdates;

    for (const auto& instruction : frameData)
    {
        auto& gc = GameState::getInstance().getComponent<CompRendering>(instruction->entityID);
        gc.entityType = instruction->entityType;
        gc.action = instruction->action;
        gc.entitySubType = instruction->entitySubType;
        gc.direction = instruction->direction;
        gc.frame = instruction->frame;
        gc.variation = instruction->variation;
        gc.positionInFeet = instruction->positionInFeet;
        gc.isStatic = instruction->isStatic;
        gc.debugOverlays = instruction->debugOverlays;

        gc.updateTextureDetails(m_graphicsRegistry);

        ObjectPool<CompGraphics>::release(instruction);
    }
    frameData.clear();
}

void Renderer::renderDebugInfo(FPSCounter& counter)
{
    // spdlog::debug("Rendering debug info...");

    addDebugText("Average FPS        : " + std::to_string(counter.getAverageFPS()));
    addDebugText("Avg Sleep/frame    : " + std::to_string(counter.getAverageSleepMs()));
    addDebugText("Viewport           : " + m_coordinates.getViewportPositionInPixels().toString());
    addDebugText("Anchor Tile        : " + m_anchorTilePixelsPos.toString());
    addDebugText("Mouse clicked feet : " + m_lastMouseClickPosInFeet.toString());
    addDebugText("Mouse clicked tile : " + m_lastMouseClickPosInTiles.toString());
    addDebugText("Graphics loaded    : " + std::to_string(m_graphicsRegistry.getTextureCount()));

    SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); /* white, full alpha */

    int y = 10;
    for (const auto& line : m_debugTexts)
    {
        SDL_RenderDebugText(m_renderer, 10, y, line.c_str());
        y += 20;
    }

    clearDebugTexts();
}

Vec2d getDebugOverlayPosition(DebugOverlay::Anchor anchor, const SDL_FRect& rect)
{
    int x = rect.x;
    int y = rect.y;
    int w = rect.w;
    int h = rect.h;

    switch (anchor)
    {
    case DebugOverlay::Anchor::TOP_LEFT:
        return {x, y};
        break;
    case DebugOverlay::Anchor::TOP_CENTER:
        return {x + w / 2, y};
        break;
    case DebugOverlay::Anchor::TOP_RIGHT:
        return {x + w, y};
        break;
    case DebugOverlay::Anchor::CENTER_LEFT:
        return {x, y + h / 2};
        break;
    case DebugOverlay::Anchor::CENTER:
        return {x + w / 2, y + h / 2};
        break;
    case DebugOverlay::Anchor::CENTER_RIGHT:
        return {x + w, y + h / 2};
        break;
    case DebugOverlay::Anchor::BOTTOM_LEFT:
        return {x, y + h};
        break;
    case DebugOverlay::Anchor::BOTTOM_CENTER:
        return {x + w / 2, y + h};
        break;
    case DebugOverlay::Anchor::BOTTOM_RIGHT:
        return {x + w, y + h};
        break;
    }
    return {x, y};
}

void Renderer::renderGameEntities()
{
    ++m_zBucketVersion; // it will take 4 trillion years to overflow this

    // TODO: Introduction of this z ordering cost around 60ms for 2500 entities.
    static bool isFirst = true;

    GameState::getInstance().getEntities<CompRendering>().each(
        [this](CompRendering& gc)
        {
            int zOrder = gc.positionInFeet.y + gc.positionInFeet.x;
            if (zOrder < 0 || zOrder >= m_zBucketsSize)
            {
                spdlog::error("Z-order out of bounds: {}", zOrder);
                return;
            }

            if (m_zBucketVersion != m_zBuckets[zOrder].version)
            {
                m_zBuckets[zOrder].version = m_zBucketVersion;
                m_zBuckets[zOrder].graphicsComponents.clear();
            }

            m_zBuckets[zOrder].graphicsComponents.push_back(&gc);
        });

    SDL_FRect dstRect = {0, 0, 0, 0};

    for (int z = 0; z < m_zBucketsSize; ++z)
    {
        if (m_zBuckets[z].version != m_zBucketVersion)
        {
            continue; // Skip if the version doesn't match
        }

        for (auto& gc : m_zBuckets[z].graphicsComponents)
        {
            auto screenPos = m_coordinates.feetToScreenUnits(gc->positionInFeet) - gc->anchor;

            // TODO: Remove this testing code
            if (isFirst)
            {
                m_anchorTilePixelsPos = m_coordinates.feetToPixels(gc->positionInFeet);
            }

            dstRect.x = screenPos.x;
            dstRect.y = screenPos.y;
            dstRect.w = gc->size.width;
            dstRect.h = gc->size.height;

            if (!gc->isStatic || m_showStaticEntities)
            {
                SDL_RenderTextureRotated(m_renderer, gc->texture, gc->srcRect, &dstRect, 0, nullptr,
                                         gc->flip);
            }

            if (m_showDebugInfo)
            {
                for (auto& overlay : gc->debugOverlays)
                {
                    auto pos = getDebugOverlayPosition(overlay.anchor, dstRect);

                    switch (overlay.type)
                    {
                    case DebugOverlay::Type::CIRCLE:
                        ellipseRGBA(m_renderer, pos.x, pos.y, 30, 15, 255, 0, 0,
                                    255); // green circle
                        break;
                    case DebugOverlay::Type::FILLED_CIRCLE:
                        filledEllipseRGBA(m_renderer, pos.x, pos.y, 20, 10, 0, 0, 255,
                                          100); // blue ellipse
                        break;
                    }
                }
            }
        }
    }
    isFirst = false;
}

void Renderer::renderBackground()
{
    SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
    SDL_RenderClear(m_renderer);
}

void aion::Renderer::renderSelectionBox()
{
    if (m_isSelecting && m_selectionStartPosScreenUnits != m_selectionEndPosScreenUnits)
    {
        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, 255);
        SDL_FRect rect = {
            (float) m_selectionStartPosScreenUnits.x, (float) m_selectionStartPosScreenUnits.y,
            (float) (m_selectionEndPosScreenUnits.x - m_selectionStartPosScreenUnits.x),
            (float) (m_selectionEndPosScreenUnits.y - m_selectionStartPosScreenUnits.y)};

        SDL_RenderRect(m_renderer, &rect);
    }
}

void Renderer::generateTicks()
{
    const auto tickDelay = milliseconds(1000 / m_settings->getTicksPerSecond());
    auto now = steady_clock::now();
    if (now - m_lastTickTime >= tickDelay)
    {
        m_lastTickTime = now;
        m_tickCount++;
        onTick();
    }
}

void Renderer::onTick()
{
    handleViewportMovement();
    m_synchronizer.getReceiverFrameData().viewportPositionInPixels =
        m_coordinates.getViewportPositionInPixels();
}

void Renderer::handleViewportMovement()
{
    auto keyStates = SDL_GetKeyboardState(nullptr);
    if (keyStates[SDL_SCANCODE_W])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() -
            Vec2d(0, m_settings->getViewportMovingSpeed()));
    }
    if (keyStates[SDL_SCANCODE_A])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() -
            Vec2d(m_settings->getViewportMovingSpeed(), 0));
    }
    if (keyStates[SDL_SCANCODE_S])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() +
            Vec2d(0, m_settings->getViewportMovingSpeed()));
    }
    if (keyStates[SDL_SCANCODE_D])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() +
            Vec2d(m_settings->getViewportMovingSpeed(), 0));
    }
}