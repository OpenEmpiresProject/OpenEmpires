#include "Viewport.h"

#include "utils/Logger.h"

#include <SDL3/SDL_events.h>

using namespace aion;

Viewport::Viewport(std::shared_ptr<GameSettings> settings) : m_settings(std::move(settings))
{
}

Vec2d Viewport::feetToPixels(const Vec2d& feet) const
{
    int pixelsX =
        ((feet.x - feet.y) * Constants::TILE_PIXEL_WIDTH) / (2 * Constants::FEET_PER_TILE);
    int pixelsY =
        ((feet.x + feet.y) * Constants::TILE_PIXEL_HEIGHT) / (2 * Constants::FEET_PER_TILE);
    int pixelsMapCenterOffsetX =
        m_settings->getWorldSizeInTiles().width * Constants::TILE_PIXEL_WIDTH / 2;
    pixelsX += pixelsMapCenterOffsetX;

    return Vec2d(pixelsX, pixelsY);
}

Vec2d Viewport::feetToScreenUnits(const Vec2d& feet) const
{
    return pixelsToScreenUnits(feetToPixels(feet));
}

Vec2d Viewport::feetToTiles(const Vec2d& feet) const
{
    return feet / Constants::FEET_PER_TILE;
}

Vec2d aion::Viewport::tilesToFeet(const Vec2d& tiles) const
{
    return tiles * Constants::FEET_PER_TILE;
}

Vec2d aion::Viewport::getTileCenterInFeet(const Vec2d& tile) const
{
    return tilesToFeet(tile) + Vec2d(Constants::FEET_PER_TILE / 2, Constants::FEET_PER_TILE / 2);
}

int Viewport::getZOrder(const Vec2d& feet) const
{
    auto pixelPos = feetToPixels(feet);
    return pixelPos.y + pixelPos.x;
}

int Viewport::getMaxZOrder() const
{
    auto worldSize = m_settings->getWorldSize();
    // Bottom corner of the map should has the highest Z-order
    return getZOrder(Vec2d(worldSize.width, worldSize.height));
}

Vec2d Viewport::pixelsToFeet(const Vec2d& pixels) const
{
    int pixelsMapCenterOffsetX =
        m_settings->getWorldSizeInTiles().width * Constants::TILE_PIXEL_WIDTH / 2;
    int pixelsX = pixels.x - pixelsMapCenterOffsetX;
    int feetX = (((pixelsX * Constants::FEET_PER_TILE) / (Constants::TILE_PIXEL_WIDTH)) +
                 ((pixels.y * Constants::FEET_PER_TILE) / (Constants::TILE_PIXEL_HEIGHT)));
    int feetY = (((pixels.y * Constants::FEET_PER_TILE) / (Constants::TILE_PIXEL_HEIGHT)) -
                 ((pixelsX * Constants::FEET_PER_TILE) / (Constants::TILE_PIXEL_WIDTH)));

    return Vec2d(feetX, feetY);
}

Vec2d Viewport::pixelsToScreenUnits(const Vec2d& pixels) const
{
    return pixels - m_viewportPositionInPixels;
}

Vec2d Viewport::screenUnitsToPixels(const Vec2d& screenUnits) const
{
    return screenUnits + m_viewportPositionInPixels;
}

Vec2d Viewport::screenUnitsToFeet(const Vec2d& screenUnits) const
{
    return pixelsToFeet(screenUnitsToPixels(screenUnits));
}

const Vec2d& Viewport::getViewportPositionInPixels() const
{
    return m_viewportPositionInPixels;
}
void Viewport::setViewportPositionInPixels(const Vec2d& pixels)
{
    m_viewportPositionInPixels = pixels;
}

void Viewport::setViewportPositionInPixelsWithBounryChecking(const Vec2d& pixels)
{
    auto originalPos = m_viewportPositionInPixels;
    setViewportPositionInPixels(pixels);

    if (!isViewportCenterInsideMap())
    {
        setViewportPositionInPixels(originalPos);
    }
}

Vec2d Viewport::getViewportCenterInPixels() const
{
    return m_viewportPositionInPixels + Vec2d(m_settings->getWindowDimensions().width / 2,
                                              m_settings->getWindowDimensions().height / 2);
}

bool Viewport::isInsideMap(const Vec2d& pixelPos) const
{
    auto gameWorldSize = m_settings->getWorldSize();
    auto feetPos = pixelsToFeet(pixelPos);

    return feetPos.x >= 0 && feetPos.x < gameWorldSize.width && feetPos.y >= 0 &&
           feetPos.y < gameWorldSize.height;
}

bool Viewport::isViewportCenterInsideMap() const
{
    return isInsideMap(getViewportCenterInPixels());
}
