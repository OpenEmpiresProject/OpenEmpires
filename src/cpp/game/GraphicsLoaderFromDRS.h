#ifndef GRAPHICSLOADERFROMDRS_H
#define GRAPHICSLOADERFROMDRS_H

#include "GraphicsLoader.h"

namespace game
{
class GraphicsLoaderFromDRS : public core::GraphicsLoader
{
private:
    void loadAllGraphics(SDL_Renderer& renderer,
                         core::GraphicsRegistry& graphicsRegistry,
                         core::AtlasGenerator& atlasGenerator) override;
    void loadGraphics(SDL_Renderer& renderer,
                      core::GraphicsRegistry& graphicsRegistry,
                      core::AtlasGenerator& atlasGenerator,
                      const std::list<core::GraphicsID>& idsToLoad) override;
};

} // namespace game

#endif