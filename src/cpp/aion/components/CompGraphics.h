#ifndef COMPGRAPHICS_H
#define COMPGRAPHICS_H

#include "GraphicsRegistry.h"
#include "Vec2d.h"
#include "utils/WidthHeight.h"

#include <SDL3/SDL.h>
#include <entt/entity/registry.hpp>
#include <variant>
#include <vector>

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

struct GraphicAddon
{
    enum class Type : uint8_t
    {
        NONE = 0,
        CIRCLE,
        SQUARE
    };

    struct Circle
    {
        int radius = 0;
        Vec2d center = {0, 0}; // Relative to anchor. i.e. actual unit position
    };

    struct Square
    {
        int width = 0;
        int height = 0;
        Vec2d center = {0, 0};
    };

    using Data = std::variant<std::monostate, Circle, Square>;

    Type type = Type::NONE;
    Data data = std::monostate{};

    template <typename T> T getData() const
    {
        return std::get<T>(data);
    }
};

// Component will be owned by the Simulator
class CompGraphics : public GraphicsID
{
  public:
    uint32_t entityID = entt::null;
    Vec2d positionInFeet = {0, 0};
    bool isStatic = false;
    std::vector<DebugOverlay> debugOverlays;
    std::vector<GraphicAddon> addons;

    CompGraphics()
    {
        addons.reserve(2);
        debugOverlays.reserve(2);
    }
};
} // namespace aion

#endif