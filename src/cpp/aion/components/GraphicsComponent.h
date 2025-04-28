#ifndef GRAPHICSCOMPONENT_H
#define GRAPHICSCOMPONENT_H

#include "Component.h"
#include "GraphicsRegistry.h"
#include "WidthHeight.h"

#include <SDL3/SDL.h>

namespace aion
{
class GraphicsComponent : public aion::Component<GraphicsComponent>
{
  public:
    GraphicsID graphicsID;          // GraphicsID to query GraphicsRegistry to get the textures
    Vec2d positionInFeet = {0, 0};   // World logical position (to be converted to screen position)
    SDL_Texture* texture = nullptr; // Texture to be rendered
    Vec2d anchor = {0, 0};         // Anchor position in pixels
    utils::WidthHeight size = {0, 0}; // Size of the texture in pixels
    SDL_FlipMode flip = SDL_FLIP_NONE; // Flip mode for the texture

    GraphicsComponent() = default;
    GraphicsComponent(const GraphicsID& graphicsID) : graphicsID(graphicsID)
    {
    }
    GraphicsComponent(const GraphicsID& graphicsID, const Vec2d& positionInFeet)
        : graphicsID(graphicsID), positionInFeet(positionInFeet)
    {
    }
    GraphicsComponent(const GraphicsID& graphicsID, SDL_Texture* texture)
        : graphicsID(graphicsID), texture(texture)
    {
    }
    GraphicsComponent(const GraphicsID& graphicsID,
                      const Vec2d& positionInFeet,
                      SDL_Texture* texture)
        : graphicsID(graphicsID), positionInFeet(positionInFeet), texture(texture)
    {
    }

    void updateTextureDetails(const GraphicsRegistry& graphicsRegistry)
    {
        auto& entry = graphicsRegistry.getTexture(graphicsID);
        if (entry.image != nullptr)
        {
            texture = entry.image;
            anchor = entry.anchor;
            size = entry.size;
            if (entry.flip)
            {
                flip = SDL_FLIP_HORIZONTAL;
            }
        }
        else
        {
            spdlog::error("Texture not found for entity: {}", graphicsID.toString());
        }
    }
};
} // namespace aion

#endif