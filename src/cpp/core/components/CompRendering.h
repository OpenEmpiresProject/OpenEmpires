#ifndef COMPRENDERING_H
#define COMPRENDERING_H

#include "CompGraphics.h"
#include "GraphicsRegistry.h"
#include "logging/Logger.h"
#include "utils/Size.h"

#include <SDL3/SDL.h>

namespace core
{
// Component will be owned by the Renderer
class CompRendering : public CompGraphics
{
  public:
    SDL_Texture* texture = nullptr;
    Vec2 anchor = {0, 0}; // Anchor position in pixels
    SDL_FlipMode flip = SDL_FLIP_NONE;
    SDL_FRect srcRect; // Source rectangle for the texture
    int additionalZOffset = 0;

    void updateTextureDetails(const GraphicsRegistry& graphicsRegistry)
    {
        auto& entry = graphicsRegistry.getTexture(*this);
        texture = entry.image; // Image can be null, which is valid
        anchor = entry.anchor;
        srcRect = *(entry.srcRect);
        if (entry.flip)
        {
            flip = SDL_FLIP_HORIZONTAL;
        }
        else
        {
            flip = SDL_FLIP_NONE;
        }
    }

    static constexpr auto properties()
    {
        return std::tuple{/* No properties for now */};
    }
};
} // namespace core

#endif