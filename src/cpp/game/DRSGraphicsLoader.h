#ifndef GRAPHICSLOADERFROMDRS_H
#define GRAPHICSLOADERFROMDRS_H

#include "GraphicsLoader.h"

namespace game
{
class DRSGraphicsLoader : public core::GraphicsLoader
{
  private:
    void initGraphicsLoadup(SDL_Renderer& renderer,
                            core::GraphicsRegistry& graphicsRegistry,
                            core::AtlasGenerator& atlasGenerator) override;
    void loadGraphics(SDL_Renderer& renderer,
                      core::GraphicsRegistry& graphicsRegistry,
                      core::AtlasGenerator& atlasGenerator,
                      const std::list<core::GraphicsID>& idsToLoad) override;
    void loadCursor(SDL_Renderer& renderer,
                    core::GraphicsRegistry& graphicsRegistry,
                    const core::GraphicsID& cursorIdToLoad) override;
};

} // namespace game

#endif