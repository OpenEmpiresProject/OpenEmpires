#ifndef GRAPHICSLOADER_H
#define GRAPHICSLOADER_H

#include "AtlasGenerator.h"
#include "GraphicsRegistry.h"

#include <SDL3/SDL.h>

namespace ion
{
class GraphicsLoader
{
  public:
    virtual void loadAllGraphics(SDL_Renderer* renderer,
                                 GraphicsRegistry& graphicsRegistry,
                                 AtlasGenerator& atlasGenerator) = 0;
    virtual void loadGraphics(SDL_Renderer* renderer,
                              GraphicsRegistry& graphicsRegistry,
                              AtlasGenerator& atlasGenerator,
                              const std::list<GraphicsID>& idsToLoad) = 0;
    virtual ~GraphicsLoader() = default;
};
} // namespace ion

#endif