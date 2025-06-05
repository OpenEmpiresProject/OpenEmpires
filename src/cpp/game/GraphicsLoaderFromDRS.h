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
};

} // namespace game

#endif