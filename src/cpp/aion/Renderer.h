#ifndef RENDERER_H
#define RENDERER_H

#include "Coordinates.h"
#include "FrameData.h"
#include "GameSettings.h"
#include "GraphicsRegistry.h"
#include "StatsCounter.h"
#include "SubSystem.h"
#include "ThreadQueue.h"
#include "ThreadSynchronizer.h"

#include <SDL3/SDL.h>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <list>
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
             GraphicsRegistry& graphicsRegistry,
             ThreadSynchronizer<FrameData>& synchronizer);
    ~Renderer();

    SDL_Renderer* getSDLRenderer();

  private:
    // SubSystem methods
    void init() override;
    void shutdown() override;

    void initSDL();
    void threadEntry();
    void renderingLoop();
    void cleanup();
    bool handleEvents();
    void updateRenderingComponents();
    void renderDebugInfo(FPSCounter& counter);
    void renderGameEntities();
    void renderBackground();
    void renderSelectionBox();
    void addDebugText(const std::string& text);
    void clearDebugTexts();
    void generateTicks();
    void onTick();
    void handleViewportMovement();
    void addComponentToZBucket(CompRendering* comp, int zOrder);
    std::list<CompRendering*> slice(CompRendering& rc);
    void addRenderingCompToZBuckets(CompRendering* rc);

    struct ZBucketVersion
    {
        int64_t version = 0;
        std::vector<CompRendering*> graphicsComponents;
        std::list<CompRendering*> newGraphicsComponents;
    };

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

    std::vector<ZBucketVersion> m_zBuckets;
    const size_t m_zBucketsSize = 0;
    int64_t m_zBucketVersion = 0;

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
};
} // namespace aion

#endif
