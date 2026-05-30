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

    Type type = Type::CIRCLE;
    Color color = Color::RED;
    Alignment anchor = Alignment::CENTER;
    Alignment customPos1;
    Alignment customPos2;
    Vec2 arrowEnd;
    Feet absolutePosition = Feet::null; // Doesn't honour anchor
    Feet rhombusCorners[4];
    uint32_t circlePixelRadius = 20;
    bool enabled = true;
};

// Component will be owned by the Simulator
class CompGraphics : public GraphicsID
{
  public:
    Property<GraphicLayer> layer = GraphicLayer::NONE;
    Property<int> constantHeight;

  public:
    uint32_t entityID = entt::null;
    uint32_t parentEntityId = entt::null;

    /*
     *   There are 4 approaches to position a graphic.
     *   Following are in the descending order of the precedence.
     *   1. positionInFeet: Screen position will be calculated
     *       based on the viewport.
     *   2. relativePixelPosition: Relative to the parent. Offset is in pixels, but
     *       not in Feet. parentEntityId must be set along with this.
     *   3. selfRelativePixelPosition: Relative to positionInFeet. Offset is in pixels.
     *       positionInFeet must be set.
     *   4. positionInScreenUnits: Absolute screen position in pixels.
     */
    Feet positionInFeet = Feet::null;
    Vec2 positionInScreenUnits;
    Vec2 relativePixelPosition = Vec2::zero;
    Vec2 selfRelativePixelPosition = Vec2::zero;

    std::vector<DebugOverlay> debugOverlays;
    std::vector<GraphicAddon> addons;
    Color shading;
    bool isDestroyed = false;
    bool isEnabled = true;
    bool bypass = false;
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
};
} // namespace core

#endif