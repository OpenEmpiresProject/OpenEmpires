#ifndef COMPGRAPHICS_H
#define COMPGRAPHICS_H

#include "Color.h"
#include "CompBuilding.h"
#include "Feet.h"
#include "GraphicAddon.h"
#include "GraphicsRegistry.h" // For GraphicsID
#include "utils/Size.h"

#include <array>
#include <entt/entity/registry.hpp>
#include <variant>
#include <vector>

namespace core
{
struct DebugOverlay
{
    enum class Type : uint8_t
    {
        CIRCLE,
        FILLED_CIRCLE,
        RHOMBUS,
        FILLED_RHOMBUS,
        ARROW,
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
    Vec2 arrowEnd;
    Feet absolutePosition = Feet::null; // Doesn't honour anchor
    Feet rhombusCorners[4];
    uint32_t circlePixelRadius = 20;
};

enum class GraphicLayer
{
    NONE = -1,
    GROUND,
    ON_GROUND,
    ENTITIES,
    SKY,
    UI
};

inline constexpr std::array<GraphicLayer, 5> g_graphicLayersOrder{
    GraphicLayer::GROUND, GraphicLayer::ON_GROUND, GraphicLayer::ENTITIES, GraphicLayer::SKY,
    GraphicLayer::UI};

// Component will be owned by the Simulator
class CompGraphics : public GraphicsID
{
  public:
    uint32_t entityID = entt::null;
    Feet positionInFeet = Feet::null;
    Vec2 positionInScreenUnits;
    std::vector<DebugOverlay> debugOverlays;
    std::vector<GraphicAddon> addons;
    Color shading;
    bool isDestroyed = false;
    bool isEnabled = true;
    bool bypass = false;
    GraphicLayer layer = GraphicLayer::NONE;
    LandArea landArea;

    CompGraphics()
    {
        addons.reserve(2);
        debugOverlays.reserve(2);
    }

    bool isBig() const
    {
        return not landArea.tiles.empty();
    }

    static constexpr auto properties()
    {
        return std::tuple{/* No properties for now */};
    }
};
} // namespace core

#endif