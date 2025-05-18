#ifndef COMPRENDERING_H
#define COMPRENDERING_H

#include "CompGraphics.h"
#include "GraphicsRegistry.h"
#include "utils/Size.h"

#include <SDL3/SDL.h>

namespace aion
{
// Component will be owned by the Renderer
class CompRendering : public CompGraphics
{
  public:
    SDL_Texture* texture = nullptr;
    Vec2d anchor = {0, 0};     // Anchor position in pixels
    SDL_FlipMode flip = SDL_FLIP_NONE;
    SDL_FRect srcRect; // Source rectangle for the texture
    int additionalZOffset = 0;

    void updateTextureDetails(const GraphicsRegistry& graphicsRegistry)
    {
        auto& entry = graphicsRegistry.getTexture(*this);
        if (entry.image != nullptr)
        {
            texture = entry.image;
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
        else
        {
            spdlog::error("Texture not found for entity: {}", GraphicsID::toString());
        }
    }
};
} // namespace aion

#endif