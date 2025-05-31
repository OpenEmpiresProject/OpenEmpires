#include "Renderer.h"

#include "AtlasGeneratorBasic.h"
#include "Event.h"
#include "FPSCounter.h"
#include "GameState.h"
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
#include <list>
#include <thread>
namespace fs = std::filesystem;
using namespace aion;
using namespace std::chrono;
using namespace std;

Renderer::Renderer(std::stop_source* stopSource,
                   GraphicsRegistry& graphicsRegistry,
                   ThreadSynchronizer<FrameData>& synchronizer,
                   GraphicsLoader& graphicsLoader)
    : SubSystem(stopSource), m_settings(ServiceRegistry::getInstance().getService<GameSettings>()),
      m_graphicsRegistry(graphicsRegistry),
      m_coordinates(ServiceRegistry::getInstance().getService<GameSettings>()),
      m_zBucketsSize(
          ServiceRegistry::getInstance().getService<GameSettings>()->getWorldSizeInTiles().height *
          Constants::FEET_PER_TILE * 3),
      m_zBuckets(
          ServiceRegistry::getInstance().getService<GameSettings>()->getWorldSizeInTiles().height *
          Constants::FEET_PER_TILE * 3),
      m_synchronizer(synchronizer),
      m_graphicsLoader(graphicsLoader)
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
        m_renderThread.join();
    }
}

SDL_Renderer* Renderer::getSDLRenderer()
{
    std::unique_lock<std::mutex> lock(m_sdlInitMutex);
    m_sdlInitCV.wait(lock, [this] { return m_sdlInitialized; });
    return m_renderer;
}

void Renderer::addDebugText(const std::string& text)
{
    m_debugTexts.push_back(text);
}

void Renderer::clearDebugTexts()
{
    m_debugTexts.clear();
}

void Renderer::cleanup()
{
    if (m_renderer)
    {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window)
    {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }
}

void Renderer::threadEntry()
{
    initSDL();
    AtlasGeneratorBasic atlasGenerator;
    m_graphicsLoader.loadAllGraphics(m_renderer, m_graphicsRegistry, atlasGenerator);
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

    while (running)
    {
        auto frame = m_synchronizer.getReceiverFrameData().frameNumber;
        // spdlog::info("Rendering frame {}", frame);

        auto start = SDL_GetTicks();
        fpsCounter.frame();
        running = handleEvents();
        m_texturesDrew = 0;

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

        m_frameTime.addSample(SDL_GetTicks() - start);

        auto waitStart = SDL_GetTicks();
        m_synchronizer.waitForSender();
        m_waitTime.addSample(SDL_GetTicks() - waitStart);

        int delay = (1000 / m_settings->getTargetFPS()) - (SDL_GetTicks() - start);
        delay = std::max(1, delay);

        SDL_Delay(delay);

        fpsCounter.sleptFor(delay);
        fpsCounter.getTotalSleep();

        m_waitTime.resetIfCountIs(1000);
        m_frameTime.resetIfCountIs(1000);
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
        auto& rc = GameState::getInstance().getComponent<CompRendering>(instruction->entityID);
        static_cast<CompGraphics&>(rc) = *instruction;
        rc.additionalZOffset = 0;

        // Graphic addons on this entity might draw below (screen's Y) the entity. So we need to
        // consider the overall bottom most position as the Z value. Eg: Unit's selection ellipse
        // should draw on top of the below tile
        for (auto& addon : rc.addons)
        {
            switch (addon.type)
            {
            case GraphicAddon::Type::CIRCLE:
                rc.additionalZOffset = Constants::FEET_PER_TILE + 1;
                break;
            }
        }

        rc.updateTextureDetails(m_graphicsRegistry);

        if (rc.isBig())
        {
            if (rc.isDestroyed)
            {
                auto existingSubComponents = m_subRenderingByEntityId.find(rc.entityID);

                if (existingSubComponents != m_subRenderingByEntityId.end())
                {
                    for (auto& existing : existingSubComponents->second)
                    {
                        m_subRenderingComponents.remove(existing);
                        ObjectPool<CompRendering>::release(existing);
                    }
                    existingSubComponents->second.clear();
                    m_subRenderingByEntityId.erase(existingSubComponents);
                }
            }
            else
            {
                auto subComponents = slice(rc);
                auto existingSubComponents = m_subRenderingByEntityId.find(rc.entityID);

                if (existingSubComponents != m_subRenderingByEntityId.end())
                {
                    for (auto& existing : existingSubComponents->second)
                    {
                        m_subRenderingComponents.remove(existing);
                        ObjectPool<CompRendering>::release(existing);
                    }
                    existingSubComponents->second.clear();
                    existingSubComponents->second = subComponents;
                }
                else
                {
                    m_subRenderingByEntityId[rc.entityID] = subComponents;
                }
                m_subRenderingComponents.splice(m_subRenderingComponents.end(), subComponents);
            }
        }
        ObjectPool<CompGraphics>::release(instruction);
    }
    frameData.clear();
}

void Renderer::renderDebugInfo(FPSCounter& counter)
{
    // spdlog::debug("Rendering debug info...");

    addDebugText("Average FPS        : " + std::to_string(counter.getAverageFPS()));
    addDebugText("Avg Sleep/frame    : " + std::to_string(counter.getAverageSleepMs()));
    addDebugText("Avg frame time     : " + std::to_string(m_frameTime.average()));
    addDebugText("Avg wait time      : " + std::to_string(m_waitTime.average()));
    addDebugText("Textures Drew      : " + std::to_string(m_texturesDrew));
    addDebugText("Viewport           : " + m_coordinates.getViewportPositionInPixels().toString());
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

Vec2d getDebugOverlayPosition(DebugOverlay::FixedPosition anchor, const SDL_FRect& rect)
{
    int x = rect.x;
    int y = rect.y;
    int w = rect.w;
    int h = rect.h;

    switch (anchor)
    {
    case DebugOverlay::FixedPosition::TOP_LEFT:
        return {x, y};
        break;
    case DebugOverlay::FixedPosition::TOP_CENTER:
        return {x + w / 2, y};
        break;
    case DebugOverlay::FixedPosition::TOP_RIGHT:
        return {x + w, y};
        break;
    case DebugOverlay::FixedPosition::CENTER_LEFT:
        return {x, y + h / 2};
        break;
    case DebugOverlay::FixedPosition::CENTER:
        return {x + w / 2, y + h / 2};
        break;
    case DebugOverlay::FixedPosition::CENTER_RIGHT:
        return {x + w, y + h / 2};
        break;
    case DebugOverlay::FixedPosition::BOTTOM_LEFT:
        return {x, y + h};
        break;
    case DebugOverlay::FixedPosition::BOTTOM_CENTER:
        return {x + w / 2, y + h};
        break;
    case DebugOverlay::FixedPosition::BOTTOM_RIGHT:
        return {x + w, y + h};
        break;
    }
    return {x, y};
}

void renderCirlceInIsometric(SDL_Renderer* renderer,
                             Sint16 cx,
                             Sint16 cy,
                             Sint16 r,
                             Uint8 red,
                             Uint8 green,
                             Uint8 blue,
                             Uint8 alpha)
{
    // Isometric ellipse radius
    Sint16 rx = r * 2; // Horizontal radius (diameter of the circle)
    Sint16 ry = r;     // Vertical radius (half of the original)

    // Render the isometric ellipse
    ellipseRGBA(renderer, cx, cy, rx, ry, red, green, blue, alpha);
}

void Renderer::renderGameEntities()
{
    ++m_zBucketVersion; // it will take 4 trillion years to overflow this

    // Following z ordering cost around 60ms for 2500 entities.

    GameState::getInstance().getEntities<CompRendering>().each(
        [this](CompRendering& rc)
        {
            if (!rc.isBig())
            {
                addRenderingCompToZBuckets(&rc);
            }
        });

    for (auto& sub : m_subRenderingComponents)
    {
        addRenderingCompToZBuckets(sub);
    }

    SDL_FRect dstRect = {0, 0, 0, 0};

    for (int z = 0; z < m_zBucketsSize; ++z)
    {
        if (m_zBuckets[z].version != m_zBucketVersion)
        {
            continue; // Skip if the version doesn't match
        }

        for (auto& rc : m_zBuckets[z].graphicsComponents)
        {
            auto screenPos = m_coordinates.feetToScreenUnits(rc->positionInFeet) - rc->anchor;

            dstRect.x = screenPos.x;
            dstRect.y = screenPos.y;
            dstRect.w = rc->srcRect.w;
            dstRect.h = rc->srcRect.h;

            if (!rc->isStatic || m_showStaticEntities)
            {
                for (auto& addon : rc->addons)
                {
                    switch (addon.type)
                    {
                    case GraphicAddon::Type::CIRCLE:
                    {
                        const auto& circle = addon.getData<GraphicAddon::Circle>();
                        auto circleScreenPos = screenPos + rc->anchor + circle.center;

                        // TODO: Support colors if required
                        renderCirlceInIsometric(m_renderer, circleScreenPos.x, circleScreenPos.y,
                                                circle.radius, 255, 255, 255, 255);
                    }
                    break;
                    case GraphicAddon::Type::SQUARE:
                        /* code */
                        break;
                    default:
                        break;
                    }
                }
                SDL_SetTextureColorMod(rc->texture, rc->shading.r, rc->shading.g, rc->shading.b);
                SDL_RenderTextureRotated(m_renderer, rc->texture, &(rc->srcRect), &dstRect, 0,
                                         nullptr, rc->flip);
                ++m_texturesDrew;
            }

            if (m_showDebugInfo)
            {
                for (auto& overlay : rc->debugOverlays)
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
                    case DebugOverlay::Type::RHOMBUS:
                    {
                        auto end1 = getDebugOverlayPosition(overlay.customPos1, dstRect);
                        auto end2 = getDebugOverlayPosition(overlay.customPos2, dstRect);

                        // Lifting the lines by a single pixel to avoid the next tile overriding
                        // these
                        lineRGBA(m_renderer, pos.x, pos.y - 1, end1.x, end1.y - 1, 180, 180, 180,
                                 255);
                        lineRGBA(m_renderer, pos.x, pos.y - 1, end2.x, end2.y - 1, 180, 180, 180,
                                 255);
                    }
                    break;
                    }
                }
            }
        }
    }
}

void Renderer::renderBackground()
{
    SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
    SDL_RenderClear(m_renderer);
}

void Renderer::renderSelectionBox()
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

void aion::Renderer::addComponentToZBucket(CompRendering* comp, int zOrder)
{
    if (m_zBucketVersion != m_zBuckets[zOrder].version)
    {
        m_zBuckets[zOrder].version = m_zBucketVersion;
        m_zBuckets[zOrder].graphicsComponents.clear();
    }

    m_zBuckets[zOrder].graphicsComponents.push_back(comp);
}

std::list<CompRendering*> aion::Renderer::slice(CompRendering& rc)
{
    static Color colors[] = {Color::RED, Color::GREEN, Color::BLUE, Color::PURPLE, Color::YELLOW};
    std::list<CompRendering*> subComponentsToReturn;

    if (rc.landSize.width > 0 && rc.landSize.height > 0)
    {
        auto slice = ObjectPool<CompRendering>::acquire();
        *slice = rc;
        slice->srcRect.w = Constants::TILE_PIXEL_WIDTH;
        slice->srcRect.x += rc.srcRect.w / 2 - (Constants::TILE_PIXEL_WIDTH / 2);
        slice->anchor.y = slice->srcRect.h;
        slice->anchor.x = slice->srcRect.w / 2;
        slice->additionalZOffset += -1 * Constants::FEET_PER_TILE;

        subComponentsToReturn.push_back(slice);

        // Slice left
        for (size_t i = 1; i < rc.landSize.width; i++)
        {
            auto slice = ObjectPool<CompRendering>::acquire();
            *slice = rc;

            // DEBUG: Slice coloring
            // slice->shading = colors[i - 1];

            // For the left-most slice, we need to capture everything to left, but not only
            // a tile width. Because there can be small details (like shadow) just outside the
            // tile width.
            if (i == (rc.landSize.width - 1))
            {
                // Set the source texture width to capture everything to left in the left-most slice
                slice->srcRect.w = rc.srcRect.w / 2 - (Constants::TILE_PIXEL_WIDTH / 2 * i);

                // Left most sclice's left edge is image-half-width left from image-center.
                slice->anchor.x = rc.srcRect.w / 2;
            }
            else
            {
                // Distance from the center of the original texture
                slice->anchor.x = Constants::TILE_PIXEL_WIDTH / 2 * (i + 1);

                // Intermediate slices are always half of a tile width
                slice->srcRect.w = Constants::TILE_PIXEL_WIDTH / 2;

                // Move the source texture selection to right
                slice->srcRect.x = rc.srcRect.x + rc.srcRect.w / 2 - slice->anchor.x;
            }

            // auto sliceZOrder = slice->positionInFeet.y + slice->positionInFeet.x -
            //                     Constants::FEET_PER_TILE * (i + 1);
            slice->additionalZOffset += -1 * Constants::FEET_PER_TILE * (i + 1);
            subComponentsToReturn.push_back(slice);
        }

        // Slice right
        for (size_t i = 1; i < rc.landSize.height; i++)
        {
            auto slice = ObjectPool<CompRendering>::acquire();
            *slice = rc;

            // DEBUG: Slice coloring
            // slice->shading = colors[i - 1];

            // For the right-most slice, we need to capture everything to right just like left
            if (i == (rc.landSize.height - 1))
            {
                // Set the source texture width to capture everything to right in the right-most
                // slice
                slice->srcRect.w = rc.srcRect.w / 2 - (Constants::TILE_PIXEL_WIDTH / 2 * i);

                // Right most sclice's left edge is image-half-width left from image-center.
                slice->anchor.x = -1 * (rc.srcRect.w / 2 - slice->srcRect.w);

                // Move the source texture selection to right
                slice->srcRect.x = rc.srcRect.x + rc.srcRect.w / 2 - slice->anchor.x;
            }
            else
            {
                // Distance from the center of the original texture
                slice->anchor.x = -1 * Constants::TILE_PIXEL_WIDTH / 2 * i;

                // Intermediate slices are always half of a tile width
                slice->srcRect.w = Constants::TILE_PIXEL_WIDTH / 2;

                // Move the source texture selection to right
                slice->srcRect.x = rc.srcRect.x + rc.srcRect.w / 2 - slice->anchor.x;
            }
            slice->additionalZOffset += -1 * Constants::FEET_PER_TILE * (i + 1);
            subComponentsToReturn.push_back(slice);
        }
    }
    return subComponentsToReturn;
}

void Renderer::addRenderingCompToZBuckets(CompRendering* rc)
{
    if (rc->isDestroyed)
    {
        return;
    }

    int zOrder = rc->positionInFeet.y + rc->positionInFeet.x + rc->additionalZOffset;

    if (zOrder < 0 || zOrder >= m_zBucketsSize)
    {
        spdlog::error("Z-order out of bounds: {}", zOrder);
        return;
    }

    SDL_Rect viewportRect = {0, 0, m_settings->getWindowDimensions().width,
                             m_settings->getWindowDimensions().height};

    auto screenPos = m_coordinates.feetToScreenUnits(rc->positionInFeet) - rc->anchor;

    SDL_Rect dstRectInt = {screenPos.x, screenPos.y, rc->srcRect.w, rc->srcRect.h};

    // Skip any texture that doesn't overlap with viewport (i.e. outside of screen)
    // This has reduced frame rendering time from 6ms to 3ms for 1366x768 window on the
    // development setup for 50x50 map in debug mode on Windows.
    if (!SDL_HasRectIntersection(&viewportRect, &dstRectInt))
    {
        return;
    }

    if (m_zBucketVersion != m_zBuckets[zOrder].version)
    {
        m_zBuckets[zOrder].version = m_zBucketVersion;
        m_zBuckets[zOrder].graphicsComponents.clear();
    }
    m_zBuckets[zOrder].graphicsComponents.push_back(rc);
}
