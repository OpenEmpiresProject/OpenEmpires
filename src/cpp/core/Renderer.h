#ifndef RENDERER_H
#define RENDERER_H

#include "FrameData.h"
#include "SubSystem.h"
#include "ThreadSynchronizer.h"

class SDL_Renderer;

namespace core
{
class RendererImpl;
class GraphicsRegistry;
class GraphicsLoader;

class Renderer : public SubSystem
{
  public:
    Renderer(std::stop_source* stopSource,
             GraphicsRegistry& graphicsRegistry,
             ThreadSynchronizer<FrameData>& synchronizer,
             GraphicsLoader& graphicsLoader);
    ~Renderer();

    SDL_Renderer* getSDLRenderer();
    bool isReady() const;

  private:
    // SubSystem methods
    void init() override;
    void shutdown() override;

    RendererImpl* m_impl;
};
} // namespace core

#endif
