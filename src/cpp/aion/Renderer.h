#ifndef RENDERER_H
#define RENDERER_H

#include "GameSettings.h"
#include "GraphicInstruction.h"
#include "GraphicsRegistry.h"
#include "SubSystem.h"
#include "ThreadQueue.h"
#include "Viewport.h"

#include <SDL3/SDL.h>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <readerwriterqueue.h>
#include <string>
#include <thread>
#include <vector>

namespace aion
{
class FPSCounter;

class Renderer : public SubSystem
{
  public:
    Renderer(std::stop_source* stopSource,
             const GameSettings& settings,
             aion::GraphicsRegistry& graphicsRegistry,
             ThreadQueue& renderQueue,
             ThreadQueue& eventLoopQueue,
             Viewport& viewport);
    ~Renderer();

    // SubSystem methods
    void init() override;
    void shutdown() override;

    // TODO: Do we need this?
    bool loadTexture(const std::string& imagePath);

    SDL_Renderer* getSDLRenderer()
    {
        std::unique_lock<std::mutex> lock(sdlInitMutex_);
        sdlInitCV_.wait(lock, [this] { return sdlInitialized_; });
        return renderer_;
    }

  private:
    void initSDL();
    void threadEntry();
    void renderingLoop();
    void cleanup();

    bool handleEvents();
    void handleGraphicInstructions();
    void renderDebugInfo(FPSCounter& counter);
    void renderGameEntities();
    void renderBackground();

    void addDebugText(const std::string& text)
    {
        debugTexts.push_back(text);
    }

    void clearDebugTexts()
    {
        debugTexts.clear();
    }

    SDL_Window* window_ = nullptr;
    SDL_Renderer* renderer_ = nullptr;
    SDL_Texture* texture_ = nullptr;

    std::thread renderThread_;
    std::atomic<bool> running_ = false;
    const GameSettings& settings;
    aion::GraphicsRegistry& graphicsRegistry;

    std::condition_variable sdlInitCV_;
    std::mutex sdlInitMutex_;
    bool sdlInitialized_ = false;

    ThreadQueue& simulatorQueue;
    ThreadQueue& eventLoopQueue_;
    int queueSize_ = 0;
    int maxQueueSize_ = 0;

    std::vector<std::string> debugTexts;

    Viewport& viewport;

    Vec2d anchorTilePixelsPos;
};
} // namespace aion

#endif
