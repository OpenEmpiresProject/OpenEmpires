#ifndef RENDERINGCOMPONENT_H
#define RENDERINGCOMPONENT_H

#include "GraphicsComponent.h"
#include "GraphicsRegistry.h"
#include "utils/WidthHeight.h"

#include <SDL3/SDL.h>

namespace aion
{
// Component will be owned by the Renderer
class RenderingComponent : public GraphicsComponent
{
  public:
    SDL_Texture* texture = nullptr;
    Vec2d anchor = {0, 0};     // Anchor position in pixels
    WidthHeight size = {0, 0}; // Size of the texture in pixels
    SDL_FlipMode flip = SDL_FLIP_NONE;
    SDL_FRect* srcRect = nullptr; // Source rectangle for the texture

    void updateTextureDetails(const GraphicsRegistry& graphicsRegistry)
    {
        auto& entry = graphicsRegistry.getTexture(*this);
        if (entry.image != nullptr)
        {
            texture = entry.image;
            anchor = entry.anchor;
            size = entry.size;
            srcRect = entry.srcRect;
            if (entry.flip)
            {
                flip = SDL_FLIP_HORIZONTAL;
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