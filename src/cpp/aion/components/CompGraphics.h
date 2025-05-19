#ifndef COMPGRAPHICS_H
#define COMPGRAPHICS_H

#include "GraphicsRegistry.h" // For GraphicsID
#include "Vec2d.h"
#include "utils/Size.h"

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
        FILLED_CIRCLE,
        RHOMBUS
    };

    enum class Color : uint8_t
    {
        RED,
        GREEN,
        BLUE,
        GREY
    };

    enum class FixedPosition : uint8_t
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
    FixedPosition anchor = FixedPosition::CENTER;
    FixedPosition customPos1;
    FixedPosition customPos2;
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

struct Color
{
    uint8_t r = 255;
    uint8_t g = 255;
    uint8_t b = 255;

    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color PURPLE;
    static const Color YELLOW;
    static const Color NONE;
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
    Color shading;
    Size landSize{0, 0};

    CompGraphics()
    {
        addons.reserve(2);
        debugOverlays.reserve(2);
    }

    bool isBig() const
    {
        return landSize.width > 0 && landSize.height > 0;
    }
};
} // namespace aion

#endif