#include "Viewport.h"

#include <SDL3/SDL_events.h>

#include "Logger.h"

using namespace aion;

Viewport::Viewport(const GameSettings& settings) : settings(settings)
{
}

Vec2d Viewport::feetToPixels(const Vec2d& feet) const
{
    int pixelsX = ((feet.x - feet.y) * utils::Constants::TILE_PIXEL_WIDTH) /
                  (2 * utils::Constants::FEET_PER_TILE);
    int pixelsY = ((feet.x + feet.y) * utils::Constants::TILE_PIXEL_HEIGHT) /
                  (2 * utils::Constants::FEET_PER_TILE);
    int pixelsMapCenterOffsetX =
        settings.getWorldSizeInTiles().width * utils::Constants::TILE_PIXEL_WIDTH / 2;
    pixelsX += pixelsMapCenterOffsetX;

    return Vec2d(pixelsX, pixelsY);
}

Vec2d Viewport::feetToScreenUnits(const Vec2d& feet) const
{
    return pixelsToScreenUnits(feetToPixels(feet));
}

Vec2d aion::Viewport::feetToTiles(const Vec2d& feet) const
{
    return feet / utils::Constants::FEET_PER_TILE;
}

int aion::Viewport::getZOrder(const Vec2d& feet) const
{
    auto pixelPos = feetToPixels(feet);
    return pixelPos.y + pixelPos.x;
}

int aion::Viewport::getMaxZOrder() const
{
    auto worldSize = settings.getWorldSize();
    // Bottom corner of the map should has the highest Z-order
    return getZOrder(Vec2d(worldSize.width, worldSize.height));
}

Vec2d Viewport::pixelsToFeet(const Vec2d& pixels) const
{
    int pixelsMapCenterOffsetX =
        settings.getWorldSizeInTiles().width * utils::Constants::TILE_PIXEL_WIDTH / 2;
    int pixelsX = pixels.x - pixelsMapCenterOffsetX;
    int feetX =
        (((pixelsX * utils::Constants::FEET_PER_TILE) / (utils::Constants::TILE_PIXEL_WIDTH)) +
         ((pixels.y * utils::Constants::FEET_PER_TILE) / (utils::Constants::TILE_PIXEL_HEIGHT)));
    int feetY =
        (((pixels.y * utils::Constants::FEET_PER_TILE) / (utils::Constants::TILE_PIXEL_HEIGHT)) -
         ((pixelsX * utils::Constants::FEET_PER_TILE) / (utils::Constants::TILE_PIXEL_WIDTH)));

    return Vec2d(feetX, feetY);
}

Vec2d Viewport::pixelsToScreenUnits(const Vec2d& pixels) const
{
    return pixels - viewportPositionInPixels;
}

Vec2d Viewport::screenUnitsToPixels(const Vec2d& screenUnits) const
{
    return screenUnits + viewportPositionInPixels;
}

Vec2d Viewport::screenUnitsToFeet(const Vec2d& screenUnits) const
{
    return pixelsToFeet(screenUnitsToPixels(screenUnits));
}

const Vec2d& Viewport::getViewportPositionInPixels() const
{
    return viewportPositionInPixels;
}
void Viewport::setViewportPositionInPixels(const Vec2d& pixels)
{
    viewportPositionInPixels = pixels;
}

void aion::Viewport::setViewportPositionInPixelsWithBounryChecking(const Vec2d& pixels)
{
    auto originalPos = viewportPositionInPixels;
    setViewportPositionInPixels(pixels);

    if (!isViewportCenterInsideMap())
    {
        setViewportPositionInPixels(originalPos);
    }
}

Vec2d Viewport::getViewportCenterInPixels() const
{
    return viewportPositionInPixels + Vec2d(settings.getWindowDimensions().width / 2,
                                            settings.getWindowDimensions().height / 2);
}

bool Viewport::isInsideMap(const Vec2d& pixelPos) const
{
    auto gameWorldSize = settings.getWorldSize();
    auto feetPos = pixelsToFeet(pixelPos);

    return feetPos.x >= 0 && feetPos.x < gameWorldSize.width &&
            feetPos.y >= 0 && feetPos.y < gameWorldSize.height;
}

bool Viewport::isViewportCenterInsideMap() const
{
    return isInsideMap(getViewportCenterInPixels());
}
