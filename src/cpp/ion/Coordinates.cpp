#include "Coordinates.h"

#include "utils/Constants.h"
#include "utils/Logger.h"

#include <SDL3/SDL_events.h>

using namespace ion;

Coordinates::Coordinates(std::shared_ptr<GameSettings> settings)
    : m_settings(std::move(settings)), m_windowMiddle(m_settings->getWindowDimensions().width / 2,
                                                      m_settings->getWindowDimensions().height / 2)
{
}

Vec2 Coordinates::feetToPixels(const Feet& feet) const
{
    int pixelsX =
        ((feet.x - feet.y) * Constants::TILE_PIXEL_WIDTH) / (2 * Constants::FEET_PER_TILE);
    int pixelsY =
        ((feet.x + feet.y) * Constants::TILE_PIXEL_HEIGHT) / (2 * Constants::FEET_PER_TILE);
    int pixelsMapCenterOffsetX =
        m_settings->getWorldSizeInTiles().width * Constants::TILE_PIXEL_WIDTH / 2;
    pixelsX += pixelsMapCenterOffsetX;

    return Vec2(pixelsX, pixelsY);
}

Vec2 Coordinates::feetToScreenUnits(const Feet& feet) const
{
    return pixelsToScreenUnits(feetToPixels(feet));
}

Tile Coordinates::feetToTiles(const Feet& feet)
{
    return Tile(feet.x / Constants::FEET_PER_TILE, feet.y / Constants::FEET_PER_TILE);
}

Feet Coordinates::tilesToFeet(const Tile& tile)
{
    return Feet(tile.x * Constants::FEET_PER_TILE, tile.y * Constants::FEET_PER_TILE);
}

Feet Coordinates::getTileCenterInFeet(const Tile& tile)
{
    return tilesToFeet(tile) + Feet(Constants::FEET_PER_TILE / 2, Constants::FEET_PER_TILE / 2);
}

int Coordinates::getZOrder(const Feet& feet) const
{
    auto pixelPos = feetToPixels(feet);
    return pixelPos.y + pixelPos.x;
}

int Coordinates::getMaxZOrder() const
{
    auto worldSize = m_settings->getWorldSize();
    // Bottom corner of the map should has the highest Z-order
    return getZOrder(Feet(worldSize.width, worldSize.height));
}

bool Coordinates::isValidFeet(const Feet& feet) const
{
    auto worldSize = m_settings->getWorldSize();
    return feet.x >= 0 && feet.x < worldSize.width && feet.y >= 0 && feet.y < worldSize.height;
}

bool Coordinates::isValidTile(const Tile& tile) const
{
    auto worldSize = m_settings->getWorldSizeInTiles();
    return tile.x >= 0 && tile.x < worldSize.width && tile.y >= 0 && tile.y < worldSize.height;
}

Feet Coordinates::pixelsToFeet(const Vec2& pixels) const
{
    int pixelsMapCenterOffsetX =
        m_settings->getWorldSizeInTiles().width * Constants::TILE_PIXEL_WIDTH / 2;
    int pixelsX = pixels.x - pixelsMapCenterOffsetX;
    int feetX = (((pixelsX * Constants::FEET_PER_TILE) / (Constants::TILE_PIXEL_WIDTH)) +
                 ((pixels.y * Constants::FEET_PER_TILE) / (Constants::TILE_PIXEL_HEIGHT)));
    int feetY = (((pixels.y * Constants::FEET_PER_TILE) / (Constants::TILE_PIXEL_HEIGHT)) -
                 ((pixelsX * Constants::FEET_PER_TILE) / (Constants::TILE_PIXEL_WIDTH)));

    return Feet(feetX, feetY);
}

Vec2 Coordinates::pixelsToScreenUnits(const Vec2& pixels) const
{
    return pixels - m_viewportPositionInPixels;
}

Vec2 Coordinates::screenUnitsToPixels(const Vec2& screenUnits) const
{
    return screenUnits + m_viewportPositionInPixels;
}

Feet Coordinates::screenUnitsToFeet(const Vec2& screenUnits) const
{
    return pixelsToFeet(screenUnitsToPixels(screenUnits));
}

Tile ion::Coordinates::screenUnitsToTiles(const Vec2& screenUnits) const
{
    return feetToTiles(screenUnitsToFeet(screenUnits));
}

const Vec2& Coordinates::getViewportPositionInPixels() const
{
    return m_viewportPositionInPixels;
}
void Coordinates::setViewportPositionInPixels(const Vec2& pixels)
{
    m_viewportPositionInPixels = pixels;
}

Feet Coordinates::getMapCenterInFeet() const
{
    return Feet(m_settings->getWorldSize().width / 2, m_settings->getWorldSize().height / 2);
}

/**
 * @brief Sets the viewport position in pixels with boundary checking.
 *
 * Attempts to set the viewport position to the specified pixel position, ensuring that the the
 * center of window for the proposed new position remains inside the map boundaries. If the center
 * for the requested position is outside the map, the function tries to adjust the position along
 * the x or y axis to find the nearest valid position within the map. The adjustment is performed by
 * iterating in both positive and negative directions for each axis. This essentially does the edge
 * scrolling. For instance, if the viewport already shows themap's right-top edge, and player press
 * right-arrow (or any equivalent key) with the idea of moving viewport further to right beyond the
 * map's edge, this would prevent that and slide the viewport along the right-top edge achiving
 * smooth scrolling.
 *
 * @param pixelPos The desired viewport position in pixel coordinates.
 */
void Coordinates::setViewportPositionInPixelsWithBounryChecking(const Vec2& pixelPos)
{
    if (isInsideMap(pixelPos + m_windowMiddle))
    {
        setViewportPositionInPixels(pixelPos);
    }
    else
    {
        auto delta = pixelPos - m_viewportPositionInPixels;
        // If the original movement is horizontal and couldn't scroll (i.e. was outside of the map)
        // try moving viewport along the edge of the map. To do that, try adjusting delta y in both
        // directions (i.e. up and down).
        if (delta.x != 0)
        {
            for (auto direction : {1, -1})
            {
                auto newPos = m_viewportPositionInPixels + Vec2(delta.x, delta.x / 2 * direction);

                if (isInsideMap(newPos + m_windowMiddle))
                {
                    setViewportPositionInPixels(newPos);
                    return;
                }
            }
        }

        // Same approach as above, but for vertical movement failures.
        if (delta.y != 0)
        {
            for (auto direction : {1, -1})
            {
                auto newPos = m_viewportPositionInPixels + Vec2(delta.y * 2 * direction, delta.y);

                if (isInsideMap(newPos + m_windowMiddle))
                {
                    setViewportPositionInPixels(newPos);
                    return;
                }
            }
        }
    }
}

Vec2 Coordinates::getViewportCenterInPixels() const
{
    return m_viewportPositionInPixels + m_windowMiddle;
}

bool Coordinates::isInsideMap(const Vec2& pixelPos) const
{
    auto gameWorldSize = m_settings->getWorldSize();
    auto feetPos = pixelsToFeet(pixelPos);

    // NOTE: While typically less than comparison should not contain the equal sign
    // in zero base index system, this is important to have map's edge scrolling work.
    return feetPos.x >= 0 && feetPos.x <= gameWorldSize.width && feetPos.y >= 0 &&
           feetPos.y <= gameWorldSize.height;
}

bool Coordinates::isViewportCenterInsideMap() const
{
    return isInsideMap(getViewportCenterInPixels());
}
