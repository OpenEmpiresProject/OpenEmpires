#ifndef GRAPHICSCOMPONENT_H
#define GRAPHICSCOMPONENT_H

#include "Component.h"
#include "GraphicsRegistry.h"

#include <SDL3/SDL.h>

namespace aion
{
class GraphicsComponent : public aion::Component<GraphicsComponent>
{
  public:
    GraphicsID graphicsID;          // GraphicsID to query GraphicsRegistry to get the textures
    Vec2d worldPosition = {0, 0};   // World logical position (to be converted to screen position)
    SDL_Texture* texture = nullptr; // Texture to be rendered

    GraphicsComponent() = default;
    GraphicsComponent(const GraphicsID& graphicsID) : graphicsID(graphicsID)
    {
    }
    GraphicsComponent(const GraphicsID& graphicsID, const Vec2d& worldPosition)
        : graphicsID(graphicsID), worldPosition(worldPosition)
    {
    }
    GraphicsComponent(const GraphicsID& graphicsID, SDL_Texture* texture)
        : graphicsID(graphicsID), texture(texture)
    {
    }
    GraphicsComponent(const GraphicsID& graphicsID,
                      const Vec2d& worldPosition,
                      SDL_Texture* texture)
        : graphicsID(graphicsID), worldPosition(worldPosition), texture(texture)
    {
    }
};
} // namespace aion

#endif