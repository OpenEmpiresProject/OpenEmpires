#ifndef COORDINATES_H
#define COORDINATES_H

#include "GameSettings.h"
#include "Vec2d.h"

#include <memory>

namespace aion
{
class Coordinates
{
  public:
    Coordinates(std::shared_ptr<GameSettings> settings);

    Vec2d feetToPixels(const Vec2d& feet) const;
    Vec2d pixelsToFeet(const Vec2d& pixels) const;
    Vec2d pixelsToScreenUnits(const Vec2d& pixels) const;
    Vec2d screenUnitsToPixels(const Vec2d& screenUnits) const;
    Vec2d screenUnitsToFeet(const Vec2d& screenUnits) const;
    Vec2d feetToScreenUnits(const Vec2d& feet) const;
    Vec2d feetToTiles(const Vec2d& feet) const;
    Vec2d tilesToFeet(const Vec2d& tiles) const;
    Vec2d getTileCenterInFeet(const Vec2d& tile) const;
    Vec2d getMapCenterInFeet() const;
    int getZOrder(const Vec2d& feet) const;
    int getMaxZOrder() const;

    const Vec2d& getViewportPositionInPixels() const;
    void setViewportPositionInPixels(const Vec2d& pixels);
    void setViewportPositionInPixelsWithBounryChecking(const Vec2d& pixels);
    Vec2d getViewportCenterInPixels() const;
    bool isInsideMap(const Vec2d& pixelPos) const;
    bool isViewportCenterInsideMap() const;

  private:
    Vec2d m_viewportPositionInPixels;
    std::shared_ptr<GameSettings> m_settings;
};

} // namespace aion

#endif