#ifndef GRAPHICSLOADERFROMDRS_H
#define GRAPHICSLOADERFROMDRS_H

#include "GraphicsLoader.h"

namespace game
{
class GraphicsLoaderFromDRS : public aion::GraphicsLoader
{
  private:
    void loadAllGraphics(SDL_Renderer* renderer,
                         aion::GraphicsRegistry& graphicsRegistry,
                         aion::AtlasGenerator& atlasGenerator) override;
};

} // namespace game

#endif