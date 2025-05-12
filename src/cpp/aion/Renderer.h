#ifndef RENDERER_H
#define RENDERER_H

#include "GameSettings.h"
#include "GraphicsRegistry.h"
#include "SubSystem.h"
#include "ThreadQueue.h"
#include "Viewport.h"
#include "ThreadSynchronizer.h"

#include <SDL3/SDL.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <memory>
#include <readerwriterqueue.h>
#include <string>
#include <thread>
#include <vector>

namespace aion
{
class FPSCounter;
class CompRendering;

class Renderer : public SubSystem
{
  public:
    Renderer(std::stop_source* stopSource,
             std::shared_ptr<GameSettings> settings,
             GraphicsRegistry& graphicsRegistry,
             ThreadQueue& renderQueue,
             Viewport& viewport,
             ThreadSynchronizer& synchronizer);
    ~Renderer();

    // SubSystem methods
    void init() override;
    void shutdown() override;

    SDL_Renderer* getSDLRenderer()
    {
        std::unique_lock<std::mutex> lock(m_sdlInitMutex);
        m_sdlInitCV.wait(lock, [this] { return m_sdlInitialized; });
        return m_renderer;
    }

  private:
    void initSDL();
    void threadEntry();
    void renderingLoop();
    void cleanup();

    bool handleEvents();
    void updateRenderingComponents();
    void renderDebugInfo(FPSCounter& counter);
    void renderGameEntities();
    void renderBackground();

    void addDebugText(const std::string& text)
    {
        m_debugTexts.push_back(text);
    }

    void clearDebugTexts()
    {
        m_debugTexts.clear();
    }

    void generateTicks();
    void onTick();
    void handleViewportMovement();

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

    ThreadQueue& m_simulatorQueue;
    int m_queueSize = 0;
    int m_maxQueueSize = 0;

    std::vector<std::string> m_debugTexts;

    Viewport& m_viewport;

    Vec2d m_anchorTilePixelsPos;

    std::chrono::steady_clock::time_point m_lastTickTime;

    Vec2d m_lastMouseClickPosInFeet;
    Vec2d m_lastMouseClickPosInTiles;

    struct ZBucketVersion
    {
        int64_t version = 0;
        std::vector<CompRendering*> graphicsComponents;
    };

    std::vector<ZBucketVersion> m_zBuckets;
    const size_t m_zBucketsSize = 0;
    int64_t m_zBucketVersion = 0;

    int64_t m_tickCount = 0;

    bool m_showStaticEntities = true;
    bool m_showDebugInfo = false;

    ThreadSynchronizer& m_synchronizer;

};
} // namespace aion

#endif
