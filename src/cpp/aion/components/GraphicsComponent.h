#ifndef GRAPHICSCOMPONENT_H
#define GRAPHICSCOMPONENT_H

#include "GraphicsRegistry.h"
#include "Vec2d.h"
#include "utils/WidthHeight.h"

#include <SDL3/SDL.h>
#include <entt/entity/registry.hpp>

namespace aion
{
struct DebugOverlay
{
    enum class Type : uint8_t
    {
        CIRCLE,
        FILLED_CIRCLE
    };

    enum class Color : uint8_t
    {
        RED,
        GREEN,
        BLUE
    };

    enum class Anchor : uint8_t
    {
        TOP_LEFT,
        TOP_CENTER,
        TOP_RIGHT,
        CENTER_LEFT,
        CENTER,
        CENTER_RIGHT,
        BOTTOM_LEFT,
        BOTTOM_CENTER,
        BOTTOM_RIGHT
    };

    Type type = Type::CIRCLE;
    Color color = Color::RED;
    Anchor anchor = Anchor::CENTER;
};

// Component will be owned by the Simulator
class GraphicsComponent : public GraphicsID
{
  public:
    uint32_t entityID = entt::null;
    Vec2d positionInFeet = {0, 0};
    bool isStatic = false;
    std::vector<DebugOverlay> debugOverlays;
};
} // namespace aion

#endif