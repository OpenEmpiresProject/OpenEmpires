#ifndef RENDERER_H
#define RENDERER_H

#include <SDL3/SDL.h>
#include <memory>
#include <thread>
#include <string>
#include <atomic>
#include "SubSystem.h"

namespace aion
{
    class Renderer : public SubSystem
    {
    public:
        Renderer(int width, int height, const std::string& title);
        ~Renderer();
    
        // SubSystem methods
        void init() override;
        void shutdown() override;
    
        // TODO: Do we need this?
        bool loadTexture(const std::string& imagePath);
    
    private:
        void threadEntry();
        void renderingLoop();
        void initSDL();
        void cleanup();
    
        SDL_Window* window_ = nullptr;
        SDL_Renderer* renderer_ = nullptr;
        SDL_Texture* texture_ = nullptr;
    
        std::thread renderThread_;
        std::atomic<bool> running_ = false;
        int width_;
        int height_;
        std::string title;
    };
}

#endif

