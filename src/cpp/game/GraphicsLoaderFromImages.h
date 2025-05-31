#ifndef GRAPHICSLOADERBYIMAGES_H
#define GRAPHICSLOADERBYIMAGES_H

#include "AtlasGenerator.h"
#include "GraphicsLoader.h"
#include "GraphicsRegistry.h"
#include "Vec2d.h"

#include <SDL3/SDL.h>
#include <filesystem>
#include <map>

namespace aion
{
class GraphicsLoaderFromImages : public GraphicsLoader
{
  public:
    void loadAllGraphics(SDL_Renderer* renderer,
                         GraphicsRegistry& graphicsRegistry,
                         AtlasGenerator& atlasGenerator) override;
    void loadTexture(const std::filesystem::path& path);
    void unloadGraphics(const std::string& graphicsPath);
    void loadAnimations(GraphicsRegistry& graphicsRegistry);
    void loadCursors();
    void setCursor(int variation);

  private:
    void loadTextures(GraphicsRegistry& graphicsRegistry, AtlasGenerator& atlasGenerator);
    void adjustDirections(GraphicsRegistry& graphicsRegistry);
    bool isTextureFlippingNeededEntity(int entityType) const;
    bool isTextureFlippingNeededDirection(Direction direction) const;
    Direction getFlippedDirection(Direction direction) const;
    int determineEntityType(const std::filesystem::path& path);
    void createAtlasForEntityType(int entityType,
                                  const std::vector<std::filesystem::path>& paths,
                                  const std::map<std::string, Vec2d>& anchors,
                                  GraphicsRegistry& graphicsRegistry,
                                  AtlasGenerator& atlasGenerator);

    SDL_Renderer* m_renderer;
    int m_variation = 0;
    std::unordered_map<int, SDL_Cursor*> m_cursors;

}; // namespace aion
} // namespace aion

#endif