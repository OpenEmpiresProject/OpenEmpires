#ifndef COORDINATES_H
#define COORDINATES_H

#include "Feet.h"
#include "Tile.h"

#include <memory>

namespace core
{
class Settings;

class Coordinates
{
  public:
    Coordinates(std::shared_ptr<Settings> settings);

    Vec2 feetToPixels(const Feet& feet) const;
    Feet pixelsToFeet(const Vec2& pixels) const;
    Vec2 pixelsToScreenUnits(const Vec2& pixels) const;
    Vec2 screenUnitsToPixels(const Vec2& screenUnits) const;
    Feet screenUnitsToFeet(const Vec2& screenUnits) const;
    Tile screenUnitsToTiles(const Vec2& screenUnits) const;
    Vec2 feetToScreenUnits(const Feet& feet) const;
    static Tile feetToTiles(const Feet& feet);
    static Feet tilesToFeet(const Tile& tiles);
    static Feet getTileCenterInFeet(const Tile& tile);
    Feet getMapCenterInFeet() const;
    int getZOrder(const Feet& feet) const;
    int getMaxZOrder() const;
    bool isValidFeet(const Feet& feet) const;
    bool isValidTile(const Tile& tile) const;

    const Vec2& getViewportPositionInPixels() const;
    void setViewportPositionInPixels(const Vec2& pixels);
    void setViewportPositionInPixelsWithBounryChecking(const Vec2& pixels);
    Vec2 getViewportCenterInPixels() const;
    bool isInsideMap(const Vec2& pixelPos) const;
    bool isViewportCenterInsideMap() const;

  private:
    Vec2 m_viewportPositionInPixels;
    std::shared_ptr<Settings> m_settings;
    const Vec2 m_windowMiddle;
};

} // namespace core

#endif