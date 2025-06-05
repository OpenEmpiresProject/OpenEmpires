#ifndef GRAPHICSLOADERBYIMAGES_H
#define GRAPHICSLOADERBYIMAGES_H

#include "AtlasGenerator.h"
#include "GraphicsLoader.h"
#include "GraphicsRegistry.h"
#include "Vec2d.h"

#include <SDL3/SDL.h>
#include <filesystem>
#include <map>

namespace game
{
class GraphicsLoaderFromImages : public aion::GraphicsLoader
{
  public:
    void loadAllGraphics(SDL_Renderer* renderer,
                         aion::GraphicsRegistry& graphicsRegistry,
                         aion::AtlasGenerator& atlasGenerator) override;
    void loadTexture(const std::filesystem::path& path);
    void unloadGraphics(const std::string& graphicsPath);
    void loadAnimations(aion::GraphicsRegistry& graphicsRegistry);
    void loadCursors();
    void setCursor(int variation);

  private:
    void loadTextures(aion::GraphicsRegistry& graphicsRegistry,
                      aion::AtlasGenerator& atlasGenerator);
    void adjustDirections(aion::GraphicsRegistry& graphicsRegistry);
    bool isTextureFlippingNeededEntity(int entityType) const;
    bool isTextureFlippingNeededDirection(aion::Direction direction) const;
    aion::Direction getFlippedDirection(aion::Direction direction) const;
    int determineEntityType(const std::filesystem::path& path);
    void createAtlasForEntityType(int entityType,
                                  const std::vector<std::filesystem::path>& paths,
                                  const std::map<std::string, aion::Vec2d>& anchors,
                                  aion::GraphicsRegistry& graphicsRegistry,
                                  aion::AtlasGenerator& atlasGenerator);

    SDL_Renderer* m_renderer;
    int m_variation = 0;
    std::unordered_map<int, SDL_Cursor*> m_cursors;
};
} // namespace game

#endif