#include "GizmoData.h"

#include "Coordinates.h"
#include "RenderingContext.h"
#include "SDL3_gfxPrimitives.h"
#include "components/CompRendering.h"

using namespace core;

extern void renderCirlceInIsometric(SDL_Renderer* renderer,
                                    Sint16 cx,
                                    Sint16 cy,
                                    Sint16 ry,
                                    Uint8 red,
                                    Uint8 green,
                                    Uint8 blue,
                                    Uint8 alpha);
extern void renderCirlceInIsometricFilled(SDL_Renderer* renderer,
                                          Sint16 cx,
                                          Sint16 cy,
                                          Sint16 ry,
                                          Uint8 red,
                                          Uint8 green,
                                          Uint8 blue,
                                          Uint8 alpha);
extern void drawFilledQuad(SDL_Renderer* renderer,
                           const core::Vec2& v0,
                           const core::Vec2& v1,
                           const core::Vec2& v2,
                           const core::Vec2& v3,
                           const core::Color& color);

void GizmoData::LineGizmo::onRender(const RenderingContext& context, const CompRendering& comp)
{
}

void GizmoData::ArrowGizmo::onRender(const RenderingContext& context, const CompRendering& comp)
{
    if (not enabled)
        return;

    Vec2 arrowStart = Vec2::null;

    if (start.isNull())
        arrowStart = comp.anchorAdjustedScreenPos + comp.anchor;
    else
        throw std::logic_error("Arrow with custom start position is not implemented");

    // TODO: Handle built-in end as well

    Vec2 arrowEnd = Vec2::null;

    if (end.isNull())
    {
        if (!comp.positionInFeet.isNull())
            arrowEnd =
                context.coordinates.feetToScreenUnits((comp.positionInFeet + (direction * length)));
    }
    else
    {
        throw std::logic_error("Arrow with custom start position is not implemented");
    }

    // Arrow shaft
    lineRGBA(context.renderer, arrowStart.x, arrowStart.y, arrowEnd.x, arrowEnd.y, color.r, color.g,
             color.b, color.a);

    // Arrowhead (two lines angled from the end point)
    float dx = arrowStart.x - arrowEnd.x;
    float dy = arrowStart.y - arrowEnd.y;
    float length = std::sqrt(dx * dx + dy * dy);

    if (length > 0.001f)
    {
        float ux = dx / length;
        float uy = dy / length;

        // Rotate by ±30 degrees to get the arrowhead wings
        float angle = M_PI / 6.0f; // 30 degrees in radians
        float sinA = std::sin(angle);
        float cosA = std::cos(angle);

        // Left wing
        float lx = cosA * ux - sinA * uy;
        float ly = sinA * ux + cosA * uy;

        // Right wing
        float rx = cosA * ux + sinA * uy;
        float ry = -sinA * ux + cosA * uy;

        const float headSize = 10.0f; // arrowhead length

        lineRGBA(context.renderer, arrowEnd.x, arrowEnd.y, arrowEnd.x + lx * headSize,
                 arrowEnd.y + ly * headSize, color.r, color.g, color.b, color.a);
        lineRGBA(context.renderer, arrowEnd.x, arrowEnd.y, arrowEnd.x + rx * headSize,
                 arrowEnd.y + ry * headSize, color.r, color.g, color.b, color.a);
    }
}

void GizmoData::CircleGizmo::onRender(const RenderingContext& context, const CompRendering& comp)
{
    if (not enabled)
        return;

    Vec2 circleCenter = Vec2::null;

    if (center.isNull())
    {
        // anchorAdjustedScreenPos is already incorporating anchor (as name suggests).
        // We need to revert that to get the logical position of the drawing.
        circleCenter = comp.anchorAdjustedScreenPos + comp.anchor;
    }
    else
    {
        circleCenter = context.coordinates.feetToScreenUnits(center);
    }
    // TODO: Handle offset

    /*
     *   Our world-to-screen project looks like;
     *   screenX = (worldX - worldY) * TILE_PIXEL_WIDTH  / (2 * FEET_PER_TILE)
     *   screenY = (worldX + worldY) * TILE_PIXEL_HEIGHT / (2 * FEET_PER_TILE)
     *
     *   Then the world coordinates on the perimeter of the circle are;
     *   x = r cos θ
     *   y = r sin θ
     *
     *   Then when we project those perimeter coords to screen;
     *   sx = (r cosθ - r sinθ) * TILE_PIXEL_WIDTH  / (2 * FEET_PER_TILE)
     *   sy = (r cosθ + r sinθ) * TILE_PIXEL_HEIGHT / (2 * FEET_PER_TILE)
     *
     *   sx becomes a maximum when cosθ - sinθ is √2
     *   sy becomes a maximum when cosθ + sinθ is √2
     *
     *   So the isometric circle radius are
     *   rx = radius * TILE_PIXEL_WIDTH  / (FEET_PER_TILE * std::sqrt(2.0))
     *   ry = radius * TILE_PIXEL_HEIGHT / (FEET_PER_TILE * std::sqrt(2.0));
     *
     */
    auto yRadiusInPixels =
        radius * Constants::TILE_PIXEL_HEIGHT / (Constants::FEET_PER_TILE * std::sqrt(2.0));

    if (filled)
    {
        renderCirlceInIsometricFilled(context.renderer, circleCenter.x, circleCenter.y,
                                      yRadiusInPixels, color.r, color.g, color.b, color.a);
    }
    else
    {
        renderCirlceInIsometric(context.renderer, circleCenter.x, circleCenter.y, yRadiusInPixels,
                                color.r, color.g, color.b, color.a);
    }
}

void GizmoData::PathGizmo::onRender(const RenderingContext& context, const CompRendering& comp)
{
    if (not enabled)
        return;

    if (points.empty())
        return;

    auto it = points.begin();

    auto previousPoint = *it;
    auto previousScreenPoint = context.coordinates.feetToScreenUnits(previousPoint);

    ++it;

    while (it != points.end())
    {
        auto currentPoint = *it;
        auto currentScreenPoint = context.coordinates.feetToScreenUnits(currentPoint);

        lineRGBA(context.renderer, previousScreenPoint.x, previousScreenPoint.y,
                 currentScreenPoint.x, currentScreenPoint.y, color.r, color.g, color.b, color.a);

        previousScreenPoint = currentScreenPoint;
        ++it;
    }
}

void GizmoData::RhombusGizmo::onRender(const RenderingContext& context, const CompRendering& comp)
{
    if (not enabled)
        return;

    static Vec2 cornersInScreenUnits[4];
    cornersInScreenUnits[0] = context.coordinates.feetToScreenUnits(corners[0]);
    cornersInScreenUnits[1] = context.coordinates.feetToScreenUnits(corners[1]);
    cornersInScreenUnits[2] = context.coordinates.feetToScreenUnits(corners[2]);
    cornersInScreenUnits[3] = context.coordinates.feetToScreenUnits(corners[3]);

    if (filled)
    {
        drawFilledQuad(context.renderer, cornersInScreenUnits[0], cornersInScreenUnits[1],
                       cornersInScreenUnits[2], cornersInScreenUnits[3], color);
    }
    else
    {
        // TODO: Not the most optimal way. We should have our own version instead of this.
        lineRGBA(context.renderer, cornersInScreenUnits[0].x, cornersInScreenUnits[0].y,
                 cornersInScreenUnits[1].x, cornersInScreenUnits[1].y, color.r, color.g, color.b,
                 color.a);
        lineRGBA(context.renderer, cornersInScreenUnits[1].x, cornersInScreenUnits[1].y,
                 cornersInScreenUnits[2].x, cornersInScreenUnits[2].y, color.r, color.g, color.b,
                 color.a);
        lineRGBA(context.renderer, cornersInScreenUnits[2].x, cornersInScreenUnits[2].y,
                 cornersInScreenUnits[3].x, cornersInScreenUnits[3].y, color.r, color.g, color.b,
                 color.a);
        lineRGBA(context.renderer, cornersInScreenUnits[3].x, cornersInScreenUnits[3].y,
                 cornersInScreenUnits[0].x, cornersInScreenUnits[0].y, color.r, color.g, color.b,
                 color.a);
    }
}
