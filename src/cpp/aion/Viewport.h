#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "Constants.h"
#include "EventHandler.h"
#include "GameSettings.h"
#include "Vec2d.h"

namespace aion
{
class Viewport : public EventHandler
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
    bool isPositionChangeRequested() const;
    void syncPosition();
    void requestPositionChange(const Vec2d& delta);

  private:
    void onInit(EventLoop* eventLoop) override
    {
    }
    void onExit() override
    {
    }
    void onEvent(const Event& e) override;

    Vec2d viewportPositionInPixels;
    const GameSettings& settings;
    std::mutex positionMutex;

    bool positionChangeRequested = false;
    Vec2d requestedChange;
};

} // namespace aion

#endif