#include "Renderer.h"

#include "AtlasGeneratorBasic.h"
#include "Coordinates.h"
#include "Event.h"
#include "FPSCounter.h"
#include "GameSettings.h"
#include "GameState.h"
#include "Renderer.h"
#include "SDL3_gfxPrimitives.h"
#include "ServiceRegistry.h"
#include "StatsCounter.h"
#include "ThreadQueue.h"
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
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <filesystem>
#include <iostream>
#include <list>
#include <memory>
#include <readerwriterqueue.h>
#include <string>
#include <thread>
#include <vector>

namespace fs = std::filesystem;
using namespace ion;
using namespace std::chrono;
using namespace std;

namespace ion
{

class RendererImpl
{
  public:
    RendererImpl(std::stop_source* stopSource,
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
        centerPixels -= Vec2d(m_settings->getWindowDimensions().width / 2,
                              m_settings->getWindowDimensions().height / 2);
        m_coordinates.setViewportPositionInPixels(centerPixels);
    }

    ~RendererImpl()
    {
        cleanup();
        // IMG_Quit();
        SDL_Quit();
    }

    SDL_Renderer* getSDLRenderer()
    {
        std::unique_lock<std::mutex> lock(m_sdlInitMutex);
        m_sdlInitCV.wait(lock, [this] { return m_sdlInitialized; });
        return m_renderer;
    }

    void init()
    {
        if (!m_running)
        {
            m_running = true;
            m_renderThread = std::thread(&RendererImpl::threadEntry, this);
        }
    }

    void shutdown()
    {
        m_running = false;
        if (m_renderThread.joinable())
        {
            m_renderThread.join();
        }
    }

    void initSDL()
    {
        spdlog::info("Initializing SDL...");

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            spdlog::error("SDL_Init failed: {}", SDL_GetError());
            throw std::runtime_error("SDL_Init failed");
        }

        m_window = SDL_CreateWindow(m_settings->getTitle().c_str(),
                                    m_settings->getWindowDimensions().width,
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

    void threadEntry()
    {
        initSDL();
        AtlasGeneratorBasic atlasGenerator;
        m_graphicsLoader.loadAllGraphics(m_renderer, m_graphicsRegistry, atlasGenerator);
        renderingLoop();
    }

    void renderingLoop()
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

    void cleanup()
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

    bool handleEvents()
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
                    m_lastMouseClickPosInFeet =
                        m_coordinates.screenUnitsToFeet(mousePosScreenUnits);
                    m_lastMouseClickPosInTiles =
                        m_coordinates.feetToTiles(m_lastMouseClickPosInFeet);

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
     * an error is logged. Invalid message types or other unexpected conditions will throw
     * exceptions.
     *
     * @throws std::runtime_error If a message with an invalid type is encountered.
     */
    void updateRenderingComponents()
    {
        // spdlog::debug("Handling graphic instructions...");
        auto& frameData = m_synchronizer.getReceiverFrameData().graphicUpdates;

        for (const auto& instruction : frameData)
        {
            auto& rc = GameState::getInstance().getComponent<CompRendering>(instruction->entityID);
            static_cast<CompGraphics&>(rc) = *instruction;
            rc.additionalZOffset = 0;

            // Graphic addons on this entity might draw below (screen's Y) the entity. So we need to
            // consider the overall bottom most position as the Z value. Eg: Unit's selection
            // ellipse should draw on top of the below tile
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

            m_zOrderStrategy->preProcess(rc);

            ObjectPool<CompGraphics>::release(instruction);
        }
        frameData.clear();
    }

    void renderDebugInfo(FPSCounter& counter)
    {
        // spdlog::debug("Rendering debug info...");

        addDebugText("Average FPS        : " + std::to_string(counter.getAverageFPS()));
        addDebugText("Avg Sleep/frame    : " + std::to_string(counter.getAverageSleepMs()));
        addDebugText("Avg frame time     : " + std::to_string(m_frameTime.average()));
        addDebugText("Avg wait time      : " + std::to_string(m_waitTime.average()));
        addDebugText("Textures Drew      : " + std::to_string(m_texturesDrew));
        addDebugText("Viewport           : " +
                     m_coordinates.getViewportPositionInPixels().toString());
        addDebugText("Mouse clicked feet : " + m_lastMouseClickPosInFeet.toString());
        addDebugText("Mouse clicked tile : " + m_lastMouseClickPosInTiles.toString());
        addDebugText("Graphics loaded    : " +
                     std::to_string(m_graphicsRegistry.getTextureCount()));

        SDL_SetRenderDrawColor(m_renderer, 255, 255, 255, SDL_ALPHA_OPAQUE); /* white, full alpha */

        int y = 10;
        for (const auto& line : m_debugTexts)
        {
            SDL_RenderDebugText(m_renderer, 10, y, line.c_str());
            y += 20;
        }

        SDL_RenderDebugText(m_renderer, 10, m_settings->getWindowDimensions().height - 20,
                            OPENEMPIRES_VERSION_STRING);

        clearDebugTexts();
    }

    void renderGameEntities()
    {
        SDL_FRect dstRect = {0, 0, 0, 0};
        auto& objectsToRender = m_zOrderStrategy->zOrder(m_coordinates);

        for (auto& rc : objectsToRender)
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

        if (m_showDebugInfo)
        {
            auto windowSize = m_settings->getWindowDimensions();
            Vec2d center(windowSize.width / 2, windowSize.height / 2);
            auto horiLineStart = center - Vec2d(5, 0);
            auto vertLineStart = center - Vec2d(0, 5);

            lineRGBA(m_renderer, horiLineStart.x, horiLineStart.y, horiLineStart.x + 10, horiLineStart.y, 255, 255, 255,
                                 255);
            lineRGBA(m_renderer, vertLineStart.x, vertLineStart.y, vertLineStart.x, vertLineStart.y + 10, 255, 255, 255,
                                 255);
        }
    }

    void renderBackground()
    {
        SDL_SetRenderDrawColor(m_renderer, 30, 30, 30, 255);
        SDL_RenderClear(m_renderer);
    }

    void renderSelectionBox()
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

    void addDebugText(const std::string& text)
    {
        m_debugTexts.push_back(text);
    }

    void clearDebugTexts()
    {
        m_debugTexts.clear();
    }

    void generateTicks()
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

    void onTick()
    {
        handleViewportMovement();
        m_synchronizer.getReceiverFrameData().viewportPositionInPixels =
            m_coordinates.getViewportPositionInPixels();
    }
    void handleViewportMovement()
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
    Vec2d m_lastMouseClickPosInFeet;
    Vec2d m_lastMouseClickPosInTiles;

    int64_t m_tickCount = 0;

    bool m_showStaticEntities = true;
    bool m_showDebugInfo = false;

    ThreadSynchronizer<FrameData>& m_synchronizer;

    Vec2d m_selectionStartPosScreenUnits;
    Vec2d m_selectionEndPosScreenUnits;
    bool m_isSelecting = false;

    std::list<CompRendering*> m_subRenderingComponents;
    std::unordered_map<uint32_t, std::list<CompRendering*>> m_subRenderingByEntityId;

    StatsCounter<uint64_t> m_frameTime;
    StatsCounter<uint64_t> m_waitTime;

    size_t m_texturesDrew = 0;

    GraphicsLoader& m_graphicsLoader;

    std::unique_ptr<ZOrderStrategyBase> m_zOrderStrategy;

    std::stop_source* m_stopSource = nullptr;
};
} // namespace ion

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