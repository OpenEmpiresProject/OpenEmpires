#ifndef GRAPHICSLOADER_H
#define GRAPHICSLOADER_H

#include "GraphicsRegistry.h"

#include <SDL3/SDL.h>
#include <filesystem>

namespace aion
{
class GraphicsLoader
{
  public:
    GraphicsLoader(SDL_Renderer* renderer, GraphicsRegistry& graphicsRegistry);
    ~GraphicsLoader() = default;

    void loadAllGraphics();
    void loadTexture(const std::filesystem::path& path);
    void unloadGraphics(const std::string& graphicsPath);
    void loadAnimations();

  private:
    void loadTextures();
    void adjustDirections();
    bool isTextureFlippingNeededEntity(int entityType) const;
    bool isTextureFlippingNeededDirection(utils::Direction direction) const;
    utils::Direction getFlippedDirection(utils::Direction direction) const;
    void loadCursor(int variation);
    SDL_Renderer* renderer_;
    int variation = 0;
    GraphicsRegistry& graphicsRegistry;

    std::unordered_map<int64_t, SDL_Surface*> loadedSurfaces;

}; // namespace aion
} // namespace aion

#endif