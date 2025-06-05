#ifndef ATLASGENERATOR_H
#define ATLASGENERATOR_H

#include <SDL3/SDL.h>
#include <filesystem>
#include <vector>

namespace ion
{
class AtlasGenerator
{
  public:
    virtual ~AtlasGenerator() = default;
    virtual SDL_Texture* generateAtlas(SDL_Renderer* renderer,
                                       const std::vector<SDL_Surface*>& surfaces,
                                       std::vector<SDL_Rect>& sourceRects) = 0;
};

} // namespace ion

#endif