#ifndef GRAPHICSLOADERFROMDRS_H
#define GRAPHICSLOADERFROMDRS_H

#include "GraphicsLoader.h"

namespace game
{
class GraphicsLoaderFromDRS : public ion::GraphicsLoader
{
  private:
    void loadAllGraphics(SDL_Renderer* renderer,
                         ion::GraphicsRegistry& graphicsRegistry,
                         ion::AtlasGenerator& atlasGenerator) override;
    void loadGraphics(SDL_Renderer* renderer,
                      ion::GraphicsRegistry& graphicsRegistry,
                      ion::AtlasGenerator& atlasGenerator,
                      const std::list<ion::GraphicsID>& idsToLoad) override;
};

} // namespace game

#endif