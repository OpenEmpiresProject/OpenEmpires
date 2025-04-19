#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <memory>
#include <thread>
#include <string>
#include <atomic>
#include "SubSystem.h"
#include "GameSettings.h"
#include "GraphicsRegistry.h"
#include <condition_variable>

namespace aion
{
    class Renderer : public SubSystem
    {
    public:
        Renderer(const GameSettings& settings, aion::GraphicsRegistry& graphicsRegistry);
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
    };
}

#endif

