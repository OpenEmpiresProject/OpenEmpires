#ifndef ATLASGENERATOR_H
#define ATLASGENERATOR_H

#include <SDL3/SDL.h>
#include <filesystem>
#include <vector>

namespace aion
{
class AtlasGenerator
{
  public:
    virtual ~AtlasGenerator() = default;
    virtual SDL_Texture* generateAtlas(SDL_Renderer* renderer,
                                       int entityType,
                                       const std::vector<std::filesystem::path>& images,
                                       std::vector<SDL_Rect>& sourceRects) = 0;
};

} // namespace aion

#endif