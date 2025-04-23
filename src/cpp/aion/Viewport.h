#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "Constants.h"
#include "GameSettings.h"
#include "Vec2d.h"

namespace aion
{
class Viewport
{
  public:
    Viewport(const GameSettings& settings);

    Vec2d feetToPixels(const Vec2d& feet) const;
    Vec2d pixelsToFeet(const Vec2d& pixels) const;
    Vec2d pixelsToScreenUnits(const Vec2d& pixels) const;
    Vec2d screenUnitsToPixels(const Vec2d& screenUnits) const;
    Vec2d screenUnitsToFeet(const Vec2d& screenUnits) const;
    Vec2d feetToScreenUnits(const Vec2d& feet) const;

    const Vec2d& getViewportPositionInPixels() const;
    void setViewportPositionInPixels(const Vec2d& pixels);

  private:
    Vec2d viewportPositionInPixels;
    const GameSettings& settings;

    bool positionChangeRequested = false;
    Vec2d requestedChange;
};

} // namespace aion

#endif