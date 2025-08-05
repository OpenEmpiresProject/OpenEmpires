#include "Renderer.h"

#include "AtlasGeneratorBasic.h"
#include "Coordinates.h"
#include "EventLoop.h"
#include "FPSCounter.h"
#include "GameSettings.h"
#include "GameState.h"
#include "GraphicsLoader.h"
#include "GraphicsRegistry.h"
#include "SDL3_gfxPrimitives.h"
#include "ServiceRegistry.h"
#include "StatsCounter.h"
#include "ThreadQueue.h"
#include "Tile.h"
#include "Version.h"
#include "ZOrderStrategyBase.h"
#include "ZOrderStrategyWithSlicing.h"
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
#include <SDL3_ttf/SDL_ttf.h>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <list>
#include <memory>
#include <readerwriterqueue.h>
#include <stdio.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;
using namespace core;
using namespace std::chrono;
using namespace std;

struct FontAtlas
{
    SDL_Texture* texture = nullptr;                // final atlas texture
    std::unordered_map<char, SDL_Rect> glyphRects; // source rects per glyph
    int atlasWidth = 0;
    int atlasHeight = 0;
};

FontAtlas createFontAtlas(SDL_Renderer* renderer, TTF_Font* font, int padding = 2)
{
    const int startChar = 32;
    const int endChar = 126;

    std::unordered_map<char, SDL_Surface*> glyphSurfaces;
    SDL_Surface* someSurface = nullptr;
    int maxGlyphHeight = 0, totalWidth = 0;

    // Step 1: Render each character to individual surfaces
    for (char c = startChar; c <= endChar; ++c)
    {
        // if (!TTF_GlyphIsProvided(font, c)) continue;

        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* glyphSurface = TTF_RenderGlyph_Blended(font, c, white);
        if (!glyphSurface)
            continue;

        someSurface = glyphSurface;
        glyphSurfaces[c] = glyphSurface;
        maxGlyphHeight = std::max(maxGlyphHeight, glyphSurface->h);
        totalWidth += glyphSurface->w + padding;
    }

    // Step 2: Create a surface to hold the full atlas
    SDL_Surface* atlasSurface = SDL_CreateSurface(totalWidth, maxGlyphHeight, someSurface->format);
    if (!atlasSurface)
    {
        spdlog::error("Failed to create atlas surface. {}", SDL_GetError());
        return FontAtlas();
    }

    // Set the palette to the atlas surface
    // if (!SDL_SetSurfacePalette(atlasSurface, SDL_GetSurfacePalette(someSurface)))
    // {
    //     spdlog::warn("Failed to set palette for atlas surface: {}", SDL_GetError());
    //     return;
    // }

    // SDL_Surface* atlasSurface = SDL_CreateRGBSurfaceWithFormat(0, totalWidth, maxGlyphHeight, 32,
    // SDL_PIXELFORMAT_RGBA32);
    if (!atlasSurface)
    {
        // Cleanup before early return
        for (auto& [c, s] : glyphSurfaces)
            SDL_DestroySurface(s);
        return {};
    }

    // Step 3: Blit each glyph onto the atlas surface
    std::unordered_map<char, SDL_Rect> srcRects;
    int xOffset = 0;
    for (char c = startChar; c <= endChar; ++c)
    {
        auto it = glyphSurfaces.find(c);
        if (it == glyphSurfaces.end())
            continue;

        SDL_Surface* glyphSurface = it->second;

        SDL_Rect dst = {xOffset, 0, glyphSurface->w, glyphSurface->h};
        SDL_BlitSurface(glyphSurface, nullptr, atlasSurface, &dst);
        srcRects[c] = dst;

        xOffset += glyphSurface->w + padding;
        SDL_DestroySurface(glyphSurface);
    }

    // Step 4: Convert atlas surface to texture
    SDL_Texture* atlasTexture = SDL_CreateTextureFromSurface(renderer, atlasSurface);
    FontAtlas result = {.texture = atlasTexture,
                        .glyphRects = std::move(srcRects),
                        .atlasWidth = atlasSurface->w,
                        .atlasHeight = atlasSurface->h};

    SDL_DestroySurface(atlasSurface);
    return result;
}

namespace core
{

class RendererImpl
{
  public:
    RendererImpl(std::stop_source* stopSource,
                 GraphicsRegistry& graphicsRegistry,
                 ThreadSynchronizer<FrameData>& synchronizer,
                 GraphicsLoader& graphicsLoader);
    ~RendererImpl();

    SDL_Renderer* getSDLRenderer();
    void init();
    void shutdown();
    void initSDL();
    void threadEntry();
    void renderingLoop();
    void cleanup();
    bool handleEvents();
    void updateRenderingComponents();
    void renderDebugInfo(FPSCounter& counter);
    void renderGameEntities();
    void renderText(const Vec2& screenPos, const std::string& text, const Color& color);
    void renderGraphicAddons(const Vec2& screenPos, CompRendering* rc);
    void renderDebugOverlays(const SDL_FRect& dstRect, CompRendering* rc);
    void renderBackground();
    void renderSelectionBox();
    void addDebugText(const std::string& text);
    void clearDebugTexts();
    void generateTicks();
    void onTick();
    void handleViewportMovement();
    Vec2 getDebugOverlayPosition(DebugOverlay::FixedPosition anchor, const SDL_FRect& rect);
    void renderCirlceInIsometric(SDL_Renderer* renderer,
                                 Sint16 cx,
                                 Sint16 cy,
                                 Sint16 r,
                                 Uint8 red,
                                 Uint8 green,
                                 Uint8 blue,
                                 Uint8 alpha);
    void loadFonts();
    bool isReady() const;

    SDL_Window* m_window = nullptr;
    SDL_Renderer* m_renderer = nullptr;
    SDL_Texture* m_texture = nullptr;

    std::thread m_renderThread;
    std::atomic<bool> m_running = false;
    std::shared_ptr<GameSettings> m_settings;
    GraphicsRegistry& m_graphicsRegistry;

    std::condition_variable m_sdlInitCV;
    std::mutex m_sdlInitMutex;
    bool m_sdlInitialized = false;

    std::vector<std::string> m_debugTexts;
    Coordinates m_coordinates;

    std::chrono::steady_clock::time_point m_lastTickTime;
    Feet m_lastMouseClickPosInFeet;
    Tile m_lastMouseClickPosInTiles;

    int64_t m_tickCount = 0;

    bool m_showDebugInfo = false;

    ThreadSynchronizer<FrameData>& m_synchronizer;

    Vec2 m_selectionStartPosScreenUnits;
    Vec2 m_selectionEndPosScreenUnits;
    bool m_isSelecting = false;

    std::list<CompRendering*> m_subRenderingComponents;
    std::unordered_map<uint32_t, std::list<CompRendering*>> m_subRenderingByEntityId;

    StatsCounter<uint64_t> m_frameTime;
    StatsCounter<uint64_t> m_waitTime;

    size_t m_texturesDrew = 0;

    GraphicsLoader& m_graphicsLoader;

    std::unique_ptr<ZOrderStrategyBase> m_zOrderStrategy;

    std::stop_source* m_stopSource = nullptr;

    FontAtlas m_fontAtlas;

    bool m_showFogOfWar = true;

    volatile bool m_isReady = false;
};
} // namespace core

RendererImpl::RendererImpl(std::stop_source* stopSource,
                           GraphicsRegistry& graphicsRegistry,
                           ThreadSynchronizer<FrameData>& synchronizer,
                           GraphicsLoader& graphicsLoader)
    : m_stopSource(stopSource),
      m_settings(ServiceRegistry::getInstance().getService<GameSettings>()),
      m_graphicsRegistry(graphicsRegistry),
      m_coordinates(ServiceRegistry::getInstance().getService<GameSettings>()),
      m_synchronizer(synchronizer), m_graphicsLoader(graphicsLoader),
      m_zOrderStrategy(std::move(std::make_unique<ZOrderStrategyWithSlicing>()))
{
    m_running = false;
    m_lastTickTime = steady_clock::now();
    spdlog::info("Max z-order: {}", m_coordinates.getMaxZOrder());

    auto center = m_coordinates.getMapCenterInFeet();
    auto centerPixels = m_coordinates.feetToPixels(center);
    centerPixels -= Vec2(m_settings->getWindowDimensions().width / 2,
                         m_settings->getWindowDimensions().height / 2);
    m_coordinates.setViewportPositionInPixels(centerPixels);
}

RendererImpl::~RendererImpl()
{
    cleanup();
    // IMG_Quit();
    SDL_Quit();
}

SDL_Renderer* RendererImpl::getSDLRenderer()
{
    std::unique_lock<std::mutex> lock(m_sdlInitMutex);
    m_sdlInitCV.wait(lock, [this] { return m_sdlInitialized; });
    return m_renderer;
}

void RendererImpl::init()
{
    if (!m_running)
    {
        m_running = true;
        m_renderThread = std::thread(&RendererImpl::threadEntry, this);
    }
}

void RendererImpl::shutdown()
{
    m_running = false;
    if (m_renderThread.joinable())
    {
        m_renderThread.join();
    }
}

void RendererImpl::initSDL()
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

    loadFonts();

    m_sdlInitCV.notify_all();
    spdlog::info("SDL initialized successfully");
}

bool RendererImpl::isReady() const
{
    return m_isReady;
}

void RendererImpl::threadEntry()
{
    initSDL();
    AtlasGeneratorBasic atlasGenerator;
    m_graphicsLoader.loadAllGraphics(m_renderer, m_graphicsRegistry, atlasGenerator);
    renderingLoop();
}

void RendererImpl::renderingLoop()
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

        if (!EventLoop::isPaused())
            m_synchronizer.waitForSender();
        m_waitTime.addSample(SDL_GetTicks() - waitStart);

        int delay = (1000 / m_settings->getTargetFPS()) - (SDL_GetTicks() - start);
        delay = std::max(1, delay);

        SDL_Delay(delay);

        fpsCounter.sleptFor(delay);
        fpsCounter.getTotalSleep();

        m_waitTime.resetIfCountIs(1000);
        m_frameTime.resetIfCountIs(1000);

        m_isReady = true;
    }

    spdlog::info("Shutting down renderer...");

    SDL_DestroyWindow(m_window);
    SDL_Quit();
    m_synchronizer.shutdown();
    m_stopSource->request_stop();
}

void RendererImpl::cleanup()
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

bool RendererImpl::handleEvents()
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
                Vec2 mousePosScreenUnits(event.button.x, event.button.y);
                m_lastMouseClickPosInFeet = m_coordinates.screenUnitsToFeet(mousePosScreenUnits);
                m_lastMouseClickPosInTiles = m_lastMouseClickPosInFeet.toTile();

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
            }
            else if (event.key.scancode == SDL_SCANCODE_9)
            {
                m_showDebugInfo = !m_showDebugInfo;
            }
            else if (event.key.scancode == SDL_SCANCODE_0)
            {
                m_showFogOfWar = !m_showFogOfWar;
            }
            else if (event.key.scancode == SDL_SCANCODE_P)
            {
                EventLoop::setPaused(!EventLoop::isPaused());
            }
        }
        else if (event.type == SDL_EVENT_MOUSE_MOTION)
        {
            if (m_isSelecting)
            {
                Vec2 mousePosScreenUnits(event.button.x, event.button.y);
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
 * an error is logged. Invalid message types or other unexpected conditions will throw
 * exceptions.
 *
 * @throws std::runtime_error If a message with an invalid type is encountered.
 */
void RendererImpl::updateRenderingComponents()
{
    // spdlog::debug("Handling graphic instructions...");
    auto& frameData = m_synchronizer.getReceiverFrameData().graphicUpdates;
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    std::list<GraphicsID> idsNeedToLoad;
    std::list<CompGraphics*> lazyLoadedInstructions;

    for (const auto& instruction : frameData)
    {
        auto& rc = gameState->getComponent<CompRendering>(instruction->entityID);
        static_cast<CompGraphics&>(rc) = *instruction;
        rc.additionalZOffset = 0;

        // Graphic addons on this entity might draw below (screen's Y) the entity. So we need to
        // consider the overall bottom most position as the Z value. Eg: Unit's selection
        // ellipse should draw on top of the below tile
        for (auto& addon : rc.addons)
        {
            switch (addon.type)
            {
            case GraphicAddon::Type::ISO_CIRCLE:
                rc.additionalZOffset = Constants::FEET_PER_TILE + 1;
                break;
            }
        }

        if (m_graphicsRegistry.hasTexture(rc))
        {
            rc.updateTextureDetails(m_graphicsRegistry);
            m_zOrderStrategy->preProcess(rc);
            ObjectPool<CompGraphics>::release(instruction);
        }
        else
        {
            idsNeedToLoad.push_back(rc);
            lazyLoadedInstructions.push_back(instruction);
        }
    }
    frameData.clear();

    if (idsNeedToLoad.empty() == false)
    {
        AtlasGeneratorBasic atlasGenerator;
        m_graphicsLoader.loadGraphics(m_renderer, m_graphicsRegistry, atlasGenerator,
                                      idsNeedToLoad);
        for (auto& instruction : lazyLoadedInstructions)
        {
            auto& rc = gameState->getComponent<CompRendering>(instruction->entityID);
            static_cast<CompGraphics&>(rc) = *instruction;
            rc.updateTextureDetails(m_graphicsRegistry);

            m_zOrderStrategy->preProcess(rc);
            ObjectPool<CompGraphics>::release(instruction);
        }
    }
}

void RendererImpl::renderDebugInfo(FPSCounter& counter)
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

    int y = 30;
    for (const auto& line : m_debugTexts)
    {
        SDL_RenderDebugText(m_renderer, 10, y, line.c_str());
        y += 20;
    }

    SDL_RenderDebugText(m_renderer, 10, m_settings->getWindowDimensions().height - 20,
                        OPENEMPIRES_VERSION_STRING);

    clearDebugTexts();
}

void RendererImpl::renderGameEntities()
{
    auto& objectsToRender = m_zOrderStrategy->zOrder(m_coordinates);
    auto& fogOfWar = m_synchronizer.getReceiverFrameData().fogOfWar;

    for (auto& rc : objectsToRender)
    {
        Vec2 screenPos = rc->positionInScreenUnits - rc->anchor;

        if (rc->positionInFeet.isNull() == false)
        {
            screenPos = m_coordinates.feetToScreenUnits(rc->positionInFeet) - rc->anchor;

            if (m_showFogOfWar && fogOfWar.isExplored(rc->positionInFeet.toTile()) == false)
                continue;
        }

        renderGraphicAddons(screenPos, rc);

        SDL_FRect dstRect = {screenPos.x, screenPos.y, rc->srcRect.w, rc->srcRect.h};

        if (dstRect.w > 0 && dstRect.h > 0 && rc->texture != nullptr)
        {
            SDL_SetTextureColorMod(rc->texture, rc->shading.r, rc->shading.g, rc->shading.b);
            SDL_RenderTextureRotated(m_renderer, rc->texture, &(rc->srcRect), &dstRect, 0, nullptr,
                                     rc->flip);
            ++m_texturesDrew;
        }

        renderDebugOverlays(dstRect, rc);
    }

    // Show a small cross at center of the screen.
    if (m_showDebugInfo)
    {
        auto windowSize = m_settings->getWindowDimensions();
        Vec2 center(windowSize.width / 2, windowSize.height / 2);
        auto horiLineStart = center - Vec2(5, 0);
        auto vertLineStart = center - Vec2(0, 5);

        lineRGBA(m_renderer, horiLineStart.x, horiLineStart.y, horiLineStart.x + 10,
                 horiLineStart.y, 255, 255, 255, 255);
        lineRGBA(m_renderer, vertLineStart.x, vertLineStart.y, vertLineStart.x,
                 vertLineStart.y + 10, 255, 255, 255, 255);
    }
}

void RendererImpl::renderText(const Vec2& screenPos, const std::string& text, const Color& color)
{
    int x = screenPos.x;
    for (char c : text)
    {
        SDL_Rect src = m_fontAtlas.glyphRects[c];

        SDL_FRect srcRect = {src.x, src.y, src.w, src.h};
        SDL_FRect dstRect = {x, screenPos.y, src.w, src.h};
        x += src.w + 1;

        SDL_SetTextureColorMod(m_fontAtlas.texture, color.r, color.g, color.b);
        SDL_RenderTextureRotated(m_renderer, m_fontAtlas.texture, &srcRect, &dstRect, 0, nullptr,
                                 SDL_FLIP_NONE);
    }
}

void RendererImpl::renderGraphicAddons(const Vec2& screenPos, CompRendering* rc)
{
    for (auto& addon : rc->addons)
    {
        switch (addon.type)
        {
        case GraphicAddon::Type::ISO_CIRCLE:
        {
            const auto& circle = addon.getData<GraphicAddon::IsoCircle>();
            auto circleScreenPos = screenPos + rc->anchor + circle.center;

            // TODO: Support colors if required
            renderCirlceInIsometric(m_renderer, circleScreenPos.x, circleScreenPos.y, circle.radius,
                                    255, 255, 255, 255);
        }
        break;
        case GraphicAddon::Type::RHOMBUS:
        {
            const auto& rhombus = addon.getData<GraphicAddon::Rhombus>();
            auto center = screenPos + rc->anchor;

            lineRGBA(m_renderer, center.x - rhombus.width / 2, center.y, center.x,
                     center.y - rhombus.height / 2, 255, 255, 255, 255);
            lineRGBA(m_renderer, center.x, center.y - rhombus.height / 2,
                     center.x + rhombus.width / 2, center.y, 255, 255, 255, 255);
            lineRGBA(m_renderer, center.x + rhombus.width / 2, center.y, center.x,
                     center.y + rhombus.height / 2, 255, 255, 255, 255);
            lineRGBA(m_renderer, center.x, center.y + rhombus.height / 2,
                     center.x - rhombus.width / 2, center.y, 255, 255, 255, 255);
        }
        break;
        case GraphicAddon::Type::TEXT:
        {
            const auto& textAddon = addon.getData<GraphicAddon::Text>();
            renderText(screenPos + rc->anchor, textAddon.text, textAddon.color);
        }
        break;
        default:
            break;
        }
    }
}

void RendererImpl::renderDebugOverlays(const SDL_FRect& dstRect, CompRendering* rc)
{
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
            {
                if (overlay.absolutePosition.isNull() == false)
                {
                    auto screenPos = m_coordinates.feetToScreenUnits(overlay.absolutePosition);
                    filledEllipseRGBA(m_renderer, screenPos.x, screenPos.y, 20, 10, overlay.color.r,
                                      overlay.color.g, overlay.color.b, 100);
                }
                else
                {
                    filledEllipseRGBA(m_renderer, pos.x, pos.y, 20, 10, 0, 0, 255,
                                      100); // blue ellipse
                }

                break;
            }

            case DebugOverlay::Type::RHOMBUS:
            {
                auto end1 = getDebugOverlayPosition(overlay.customPos1, dstRect);
                auto end2 = getDebugOverlayPosition(overlay.customPos2, dstRect);

                // Lifting the lines by a single pixel to avoid the next tile overriding
                // these
                lineRGBA(m_renderer, pos.x, pos.y - 1, end1.x, end1.y - 1, 180, 180, 180, 255);
                lineRGBA(m_renderer, pos.x, pos.y - 1, end2.x, end2.y - 1, 180, 180, 180, 255);
            }
            break;
            case DebugOverlay::Type::ARROW:
            {
                // TODO: Use null
                if (overlay.arrowEnd.x == 0 && overlay.arrowEnd.y == 0)
                    break;

                auto end = overlay.arrowEnd;

                // Arrow shaft
                lineRGBA(m_renderer, pos.x, pos.y, end.x, end.y, overlay.color.r, overlay.color.g,
                         overlay.color.b, 255);

                // Arrowhead (two lines angled from the end point)
                float dx = pos.x - end.x;
                float dy = pos.y - end.y;
                float length = std::sqrt(dx * dx + dy * dy);

                if (length > 0.001f)
                {
                    float ux = dx / length;
                    float uy = dy / length;

                    // Rotate by ±30 degrees to get the arrowhead wings
                    float angle = M_PI / 6.0f; // 30 degrees in radians
                    float sinA = std::sin(angle);
                    float cosA = std::cos(angle);

                    // Left wing
                    float lx = cosA * ux - sinA * uy;
                    float ly = sinA * ux + cosA * uy;

                    // Right wing
                    float rx = cosA * ux + sinA * uy;
                    float ry = -sinA * ux + cosA * uy;

                    const float headSize = 10.0f; // arrowhead length

                    lineRGBA(m_renderer, end.x, end.y, end.x + lx * headSize, end.y + ly * headSize,
                             overlay.color.r, overlay.color.g, overlay.color.b, 255);
                    lineRGBA(m_renderer, end.x, end.y, end.x + rx * headSize, end.y + ry * headSize,
                             overlay.color.r, overlay.color.g, overlay.color.b, 255);
                }
            }
            break;
            }
        }
    }
}

void RendererImpl::renderBackground()
{
    SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_renderer);
}

void RendererImpl::renderSelectionBox()
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

void RendererImpl::addDebugText(const std::string& text)
{
    m_debugTexts.push_back(text);
}

void RendererImpl::clearDebugTexts()
{
    m_debugTexts.clear();
}

void RendererImpl::generateTicks()
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

void RendererImpl::onTick()
{
    handleViewportMovement();
    m_synchronizer.getReceiverFrameData().viewportPositionInPixels =
        m_coordinates.getViewportPositionInPixels();
}
void RendererImpl::handleViewportMovement()
{
    auto keyStates = SDL_GetKeyboardState(nullptr);
    if (keyStates[SDL_SCANCODE_W])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() -
            Vec2(0, m_settings->getViewportMovingSpeed()));
    }
    if (keyStates[SDL_SCANCODE_A])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() -
            Vec2(m_settings->getViewportMovingSpeed(), 0));
    }
    if (keyStates[SDL_SCANCODE_S])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() +
            Vec2(0, m_settings->getViewportMovingSpeed()));
    }
    if (keyStates[SDL_SCANCODE_D])
    {
        m_coordinates.setViewportPositionInPixelsWithBounryChecking(
            m_coordinates.getViewportPositionInPixels() +
            Vec2(m_settings->getViewportMovingSpeed(), 0));
    }
}

Vec2 RendererImpl::getDebugOverlayPosition(DebugOverlay::FixedPosition anchor,
                                           const SDL_FRect& rect)
{
    float x = rect.x;
    float y = rect.y;
    float w = rect.w;
    float h = rect.h;

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

void RendererImpl::renderCirlceInIsometric(SDL_Renderer* renderer,
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

void RendererImpl::loadFonts()
{
    if (TTF_Init() == false)
    {
        spdlog::error("Failed to initialize TTF: {}", SDL_GetError());
        throw std::runtime_error("TTF_Init failed");
    }

    TTF_Font* font = TTF_OpenFont("assets/fonts/Roboto-Bold.ttf", 14); // 24 pt font
    if (!font)
    {
        spdlog::error("Failed to open the font: {}", SDL_GetError());
        throw std::runtime_error("TTF_OpenFont failed");
    }

    m_fontAtlas = createFontAtlas(m_renderer, font);
}

Renderer::Renderer(std::stop_source* stopSource,
                   GraphicsRegistry& graphicsRegistry,
                   ThreadSynchronizer<FrameData>& synchronizer,
                   GraphicsLoader& graphicsLoader)
    : SubSystem(stopSource)
{
    m_impl = new RendererImpl(stopSource, graphicsRegistry, synchronizer, graphicsLoader);
}

Renderer::~Renderer()
{
    delete m_impl;
}

void Renderer::init()
{
    m_impl->init();
}

void Renderer::shutdown()
{
    m_impl->shutdown();
}

SDL_Renderer* Renderer::getSDLRenderer()
{
    return m_impl->getSDLRenderer();
}

bool Renderer::isReady() const
{
    return m_impl->isReady();
}
