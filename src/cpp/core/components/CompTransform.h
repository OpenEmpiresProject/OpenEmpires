#ifndef COMPTRANSFORM_H
#define COMPTRANSFORM_H

#include "Feet.h"
#include "Property.h"
#include "Tile.h"
#include "utils/Types.h"

#include <cassert>
#include <numbers>

namespace core
{
constexpr float DEG_TO_RAD = 3.14159265f / 180.0f;

inline float toRadians(float degrees)
{
    return degrees * DEG_TO_RAD;
}

class CompTransform
{
  public:
    Property<uint32_t> speed; // Feet per second
    Property<bool> hasRotation;

  public:
    Feet position{0, 0}; // Position in feet
    int rotation = 0;    // Rotation in degrees from feet North (0 degrees is up)
    int goalRadiusSquared = 150 ^ 2;
    int goalRadius = 150;
    int selectionBoxWidth = 15;
    int selectionBoxHeight = 30;
    int collisionRadius = 100; // TODO: not every entity would be circular

    CompTransform() = default;
    CompTransform(int x, int y) : position(x, y)
    {
    }
    CompTransform(const Feet& pos) : position(pos)
    {
    }
    CompTransform(const CompTransform&) = default;
    CompTransform& operator=(const CompTransform&) = default;

    Feet getVelocityVector() const
    {
        if (!hasRotation || speed == 0)
            return {0, 0};

        auto angleDeg = static_cast<float>(rotation);
        auto angleRad = toRadians(angleDeg);

        // 0 degrees = North = -Y
        float dx = std::sin(angleRad);
        float dy = -std::cos(angleRad); // flipped to make 0° point upward (-Y)

        return {dx * speed, dy * speed};
    }

    void face(const Feet& target)
    {
        // Calculate the angle to face the target position
        int deltaX = target.x - position.x;
        int deltaY = target.y - position.y;
        rotation = static_cast<int>(std::atan2(deltaX, -deltaY) * 180 / std::numbers::pi);

        if (rotation < 0)
        {
            rotation += 360.0;
        }
    }

    void face(const Feet& relativeTo, const Feet& target)
    {
        // Calculate the angle to face the target position relative to the
        // provided position rather than our own position
        int deltaX = target.x - relativeTo.x;
        int deltaY = target.y - relativeTo.y;
        rotation = static_cast<int>(std::atan2(deltaX, -deltaY) * 180 / std::numbers::pi);

        if (rotation < 0)
        {
            rotation += 360.0;
        }
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
        return Direction::NONE;
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
        return Direction::NONE;
    }

    Tile getTilePosition() const
    {
        return {static_cast<int>(position.x / Constants::FEET_PER_TILE),
                static_cast<int>(position.y / Constants::FEET_PER_TILE)};
    }
};
} // namespace core

#endif