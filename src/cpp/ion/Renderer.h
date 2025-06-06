#ifndef RENDERER_H
#define RENDERER_H

#include "FrameData.h"
#include "GraphicsLoader.h"
#include "GraphicsRegistry.h"
#include "SubSystem.h"
#include "ThreadSynchronizer.h"

#include <SDL3/SDL.h>

namespace ion
{
class RendererImpl;
class Renderer : public SubSystem
{
  public:
    Renderer(std::stop_source* stopSource,
             GraphicsRegistry& graphicsRegistry,
             ThreadSynchronizer<FrameData>& synchronizer,
             GraphicsLoader& graphicsLoader);
    ~Renderer();

    SDL_Renderer* getSDLRenderer();

  private:
    // SubSystem methods
    void init() override;
    void shutdown() override;

    RendererImpl* m_impl;
};
} // namespace ion

#endif
