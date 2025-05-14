#ifndef COMPTRANSFORM_H
#define COMPTRANSFORM_H

#include "Vec2d.h"
#include "utils/Types.h"

#include <numbers>

namespace aion
{
class CompTransform
{
  public:
    Vec2d position{0, 0}; // Position in feet
    int rotation = 0;     // Rotation in degrees from feet North (0 degrees is up)
    bool hasRotation = false;
    int speed = 0; // Speed in feet per second
    int goalRadiusSquared = 100;
    int selectionBoxWidth = 15;
    int selectionBoxHeight = 30;
    int collisionRadius = 10; // TODO: not every entity would be circular
    // int prevRotation = 0;

    CompTransform() = default;
    CompTransform(int x, int y) : position(x, y)
    {
    }
    CompTransform(const Vec2d& pos) : position(pos)
    {
    }
    CompTransform(const CompTransform&) = default;
    CompTransform& operator=(const CompTransform&) = default;

    void face(const Vec2d& target)
    {
        // Calculate the angle to face the target position
        int deltaX = target.x - position.x;
        int deltaY = target.y - position.y;
        rotation = static_cast<int>(std::atan2(deltaX, -deltaY) * 180 / std::numbers::pi);

        if (rotation < 0)
        {
            rotation += 360.0;
        }

        // if (prevRotation != rotation)
        // {
        //     spdlog::debug("current rotation: {}, current pos {}, target {}", rotation,
        //     position.toString(), target.toString()); auto dir = getIsometricDirection();
        //     spdlog::debug("rotation: {}, dir: {}", rotation, static_cast<int>(dir));
        //     prevRotation = rotation;
        // }
    }

    void walk(int timeMs)
    {
        assert(speed > 0 && "Speed must be greater than zero");
        // Update the position based on speed and time
        auto rad = (std::numbers::pi * rotation) / 180;
        auto timeS = (double) timeMs / 1000.0;
        position.y -= speed * std::cos(rad) * timeS;
        position.x += speed * std::sin(rad) * timeS;
    }

    Direction getIsometricDirection() const
    {
        if (hasRotation)
        {
            // Convert rotation to isometric direction
            // int isoRotation = (rotation + 45) % 360; // Adjust for isometric view
            int isoRotation = static_cast<int>(std::round((rotation + 45) / 45.0)) % 8;
            return static_cast<Direction>(isoRotation);
        }
        else
        {
            return Direction::NONE;
        }
    }

    void face(Direction direction)
    {
        rotation = 45 * static_cast<int>(direction);
    }

    Direction getDirection() const
    {
        if (hasRotation)
        {
            return static_cast<Direction>((rotation % 360) / 45);
        }
        else
        {
            return Direction::NONE;
        }
    }

    Vec2d getTilePosition() const
    {
        return Vec2d{static_cast<int>(position.x / Constants::FEET_PER_TILE),
                     static_cast<int>(position.y / Constants::FEET_PER_TILE)};
    }
};
} // namespace aion

#endif