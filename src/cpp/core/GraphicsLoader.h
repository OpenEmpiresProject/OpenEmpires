#ifndef GRAPHICSLOADER_H
#define GRAPHICSLOADER_H

#include "GraphicsRegistry.h"

class SDL_Renderer;

namespace core
{
class AtlasGenerator;

class GraphicsLoader
{
  public:
    virtual void initGraphicsLoadup(SDL_Renderer& renderer,
                                    GraphicsRegistry& graphicsRegistry,
                                    AtlasGenerator& atlasGenerator) = 0;
    virtual void loadGraphics(SDL_Renderer& renderer,
                              GraphicsRegistry& graphicsRegistry,
                              AtlasGenerator& atlasGenerator,
                              const std::list<GraphicsID>& idsToLoad) = 0;
    virtual void loadCursor(SDL_Renderer& renderer,
                            GraphicsRegistry& graphicsRegistry,
                            const GraphicsID& cursorIdToLoad) = 0;
    virtual ~GraphicsLoader() = default;
};
} // namespace core

#endif