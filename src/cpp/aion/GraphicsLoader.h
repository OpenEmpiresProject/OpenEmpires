#ifndef GRAPHICSLOADER_H
#define GRAPHICSLOADER_H

#include "AtlasGeneratorBase.h"
#include "GraphicsRegistry.h"

#include <SDL3/SDL.h>
#include <filesystem>

namespace aion
{
class GraphicsLoader
{
  public:
    GraphicsLoader(SDL_Renderer* renderer,
                   GraphicsRegistry& graphicsRegistry,
                   AtlasGeneratorBase& atlasGenerator);
    ~GraphicsLoader() = default;

    void loadAllGraphics();
    void loadTexture(const std::filesystem::path& path);
    void unloadGraphics(const std::string& graphicsPath);
    void loadAnimations();
    void loadCursors();
    void setCursor(int variation);

  private:
    void loadTextures();
    void adjustDirections();
    bool isTextureFlippingNeededEntity(int entityType) const;
    bool isTextureFlippingNeededDirection(Direction direction) const;
    Direction getFlippedDirection(Direction direction) const;
    int determineEntityType(const std::filesystem::path& path);
    void createAtlasForEntityType(int entityType, const std::vector<std::filesystem::path>& paths);

    SDL_Renderer* m_renderer;
    int m_variation = 0;
    GraphicsRegistry& m_graphicsRegistry;

    AtlasGeneratorBase& m_atlasGenerator;

    std::unordered_map<int, SDL_Cursor*> m_cursors;

}; // namespace aion
} // namespace aion

#endif