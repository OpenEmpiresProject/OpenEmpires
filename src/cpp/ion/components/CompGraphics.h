#ifndef COMPGRAPHICS_H
#define COMPGRAPHICS_H

#include "Color.h"
#include "GraphicAddon.h"
#include "GraphicsRegistry.h" // For GraphicsID
#include "Vec2d.h"
#include "utils/Size.h"

#include <array>
#include <entt/entity/registry.hpp>
#include <variant>
#include <vector>

namespace ion
{
struct DebugOverlay
{
    enum class Type : uint8_t
    {
        CIRCLE,
        FILLED_CIRCLE,
        RHOMBUS
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

enum class GraphicLayer
{
    NONE = -1,
    GROUND,
    ENTITIES,
    SKY,
    FOG,
    UI
};

inline constexpr std::array<GraphicLayer, 5> GraphicLayersOrder{
    GraphicLayer::GROUND, GraphicLayer::ENTITIES, GraphicLayer::SKY, GraphicLayer::FOG,
    GraphicLayer::UI};
// Component will be owned by the Simulator
class CompGraphics : public GraphicsID
{
  public:
    uint32_t entityID = entt::null;
    Vec2d positionInFeet = {0, 0};
    Vec2d positionInScreenUnits = Vec2d::null;
    std::vector<DebugOverlay> debugOverlays;
    std::vector<GraphicAddon> addons;
    Color shading;
    Size landSize{0, 0};
    bool isDestroyed = false;
    GraphicLayer layer = GraphicLayer::NONE;

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
} // namespace ion

#endif