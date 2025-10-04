#ifndef GRAPHICSLOADERBYIMAGES_H
#define GRAPHICSLOADERBYIMAGES_H

#include "AtlasGenerator.h"
#include "Feet.h"
#include "GraphicsLoader.h"
#include "GraphicsRegistry.h"

#include <SDL3/SDL.h>
#include <filesystem>
#include <map>

namespace game
{
class GraphicsLoaderFromImages : public core::GraphicsLoader
{
  public:
    void loadAllGraphics(SDL_Renderer& renderer,
                         core::GraphicsRegistry& graphicsRegistry,
                         core::AtlasGenerator& atlasGenerator) override;
    void loadCursors();
    void setCursor(int variation);

  private:
    void loadTextures(SDL_Renderer& renderer,
                      core::GraphicsRegistry& graphicsRegistry,
                      core::AtlasGenerator& atlasGenerator);
    void adjustDirections(core::GraphicsRegistry& graphicsRegistry);
    bool isTextureFlippingNeededEntity(int entityType) const;
    bool isTextureFlippingNeededDirection(core::Direction direction) const;
    core::Direction getFlippedDirection(core::Direction direction) const;
    int determineEntityType(const std::filesystem::path& path);
    void createAtlasForEntityType(SDL_Renderer& renderer,
                                  int entityType,
                                  const std::vector<std::filesystem::path>& paths,
                                  const std::map<std::string, core::Vec2>& anchors,
                                  core::GraphicsRegistry& graphicsRegistry,
                                  core::AtlasGenerator& atlasGenerator);

    int m_variation = 0;
    std::unordered_map<int, SDL_Cursor*> m_cursors;
};
} // namespace game

#endif