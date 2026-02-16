#include "GraphicAddon.h"

#include "Coordinates.h"
#include "RenderingContext.h"
#include "SDL3_gfxPrimitives.h"
#include "components/CompRendering.h"

#include <SDL3/SDL.h>

using namespace core;

namespace core
{
struct FontAtlas;
}
extern void renderCirlceInIsometric(SDL_Renderer* renderer,
                                    Sint16 cx,
                                    Sint16 cy,
                                    Sint16 r,
                                    Uint8 red,
                                    Uint8 green,
                                    Uint8 blue,
                                    Uint8 alpha);
extern void renderText(const FontAtlas& fontAtlas,
                       SDL_Renderer* renderer,
                       const Vec2& screenPos,
                       const std::string& text,
                       const Color& color);
extern Vec2 convertAlignmentToPosition(Alignment alignment, const SDL_FRect& rect);

Vec2 getAnchorAdjustedScreenPos(const CompRendering& comp, const Coordinates& coordinates)
{
    if (comp.positionInFeet.isNull()) [[unlikely]]
    {
        return comp.positionInScreenUnits - comp.anchor;
    }
    else
    {
        return coordinates.feetToScreenUnits(comp.positionInFeet) - comp.anchor;
    }
}

void GraphicAddon::IsoCircle::onRender(const RenderingContext& context,
                                       const CompRendering& comp,
                                       Alignment alignment,
                                       const Margin& margin)
{
    Vec2 anchorAdjustedScreenPos = getAnchorAdjustedScreenPos(comp, context.coordinates);
    // TODO: instead of negating anchor, use alignment to determine the offset
    auto circleScreenPos = anchorAdjustedScreenPos + comp.anchor + center;

    // TODO: Support colors if required
    // TODO: Support alignments
    renderCirlceInIsometric(context.renderer, circleScreenPos.x, circleScreenPos.y, radius, 255,
                            255, 255, 255);
}

void GraphicAddon::Square::onRender(const RenderingContext& context,
                                    const CompRendering& comp,
                                    Alignment alignment,
                                    const Margin& margin)
{
}

void GraphicAddon::Rhombus::onRender(const RenderingContext& context,
                                     const CompRendering& comp,
                                     Alignment alignment,
                                     const Margin& margin)
{
    Vec2 anchorAdjustedScreenPos = getAnchorAdjustedScreenPos(comp, context.coordinates);
    auto center = anchorAdjustedScreenPos + comp.anchor;

    lineRGBA(context.renderer, center.x - width / 2, center.y, center.x, center.y - height / 2, 255,
             255, 255, 255);
    lineRGBA(context.renderer, center.x, center.y - height / 2, center.x + width / 2, center.y, 255,
             255, 255, 255);
    lineRGBA(context.renderer, center.x + width / 2, center.y, center.x, center.y + height / 2, 255,
             255, 255, 255);
    lineRGBA(context.renderer, center.x, center.y + height / 2, center.x - width / 2, center.y, 255,
             255, 255, 255);
}

void GraphicAddon::Text::onRender(const RenderingContext& context,
                                  const CompRendering& comp,
                                  Alignment alignment,
                                  const Margin& margin)
{
    Vec2 anchorAdjustedScreenPos = getAnchorAdjustedScreenPos(comp, context.coordinates);
    renderText(context.fontAtlas, context.renderer, anchorAdjustedScreenPos + comp.anchor, text,
               color);
}

void GraphicAddon::HealthBar::onRender(const RenderingContext& context,
                                       const CompRendering& comp,
                                       Alignment alignment,
                                       const Margin& margin)
{
    Vec2 anchorAdjustedScreenPos = getAnchorAdjustedScreenPos(comp, context.coordinates);

    // For health bar, we want to maintain a constant position for a given action. For
    // instance, when villager building, texture width and anchor changes across frames.
    // Another example is when a unit is walking, the anchor goes up and down across frames.
    // Therefore, we first negate the anchor to get the top left position of the texture
    // and then calculate the screen position using given alignment.
    //
    Vec2 screenPosWithConstantHeight =
        anchorAdjustedScreenPos -
        Vec2(comp.srcRect.w / 2 - comp.anchor.x, comp.constantHeight - comp.anchor.y);

    SDL_FRect dstRect = {screenPosWithConstantHeight.x, screenPosWithConstantHeight.y,
                         comp.srcRect.w, comp.constantHeight};
    auto alignedScreenPos = convertAlignmentToPosition(alignment, dstRect);

    // TODO: Support colors if required
    // TODO: Might need changes to the offset dynamically depending on the resolution
    constexpr int barWidth = 25;
    constexpr int barHeight = 4;
    const Color missingHealthColor = Color::RED;
    const Color remainingHealth = Color::GREEN;
    SDL_FRect barRect = {alignedScreenPos.x - barWidth / 2 - margin.horizontal,
                         alignedScreenPos.y - barHeight / 2 - margin.vertical, barWidth, barHeight};
    SDL_Color colorBefore;
    SDL_GetRenderDrawColor(context.renderer, &colorBefore.r, &colorBefore.g, &colorBefore.b,
                           &colorBefore.a);

    SDL_SetRenderDrawColor(context.renderer, missingHealthColor.r, missingHealthColor.g,
                           missingHealthColor.b, missingHealthColor.a);
    SDL_RenderFillRect(context.renderer, &barRect);
    barRect.w *= percentage; // scale width by health percentage
    SDL_SetRenderDrawColor(context.renderer, remainingHealth.r, remainingHealth.g,
                           remainingHealth.b, remainingHealth.a);
    SDL_RenderFillRect(context.renderer, &barRect);
    SDL_SetRenderDrawColor(context.renderer, colorBefore.r, colorBefore.g, colorBefore.b,
                           colorBefore.a);
}
