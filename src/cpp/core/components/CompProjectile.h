#ifndef CORE_COMPPROJECTILE_H
#define CORE_COMPPROJECTILE_H
#include "Feet.h"
#include "Property.h"
#include "utils/Types.h"

#include <algorithm>

namespace core
{
constexpr float RAD_TO_DEG = 180.0f / 3.14159265f;

inline float toDegrees(float radian)
{
    return radian * RAD_TO_DEG;
}

class CompProjectile : public ProjectileProperties
{
  public:
    ProjectileDamageMode damageMode;
    int speed = 0;
    Feet originPosition = Feet::null;
    Feet targetPosition = Feet::null;
    Feet direction = Feet::zero;
    float rangeSq = 0;
    Vec2 previousPixelPos = Vec2::null;
    int releaseHeight = 0; // Pixels

    static constexpr double ARC_HEIGHT_FACTOR = 0.5;

    CompProjectile& operator=(const ProjectileProperties& rhs)
    {
        attackPerClass = rhs.attackPerClass;
        attackMultiplierPerClass = rhs.attackMultiplierPerClass;
        accuracy = rhs.accuracy;
        reloadTimeS = rhs.reloadTimeS;
        return *this;
    }

    /*
     *   Approach:
     *   Logical height of the projectile is modeled as function of normalized
     *   traveled distance (t) - i.e. as a parabolic -.
     *           height = t (1 - t)
     *   To get the max height at the logical mid point of the travel (here
     *   it is assumed every shooter send the projectile at 45 degrees optimal
     *   angle), it is required to multiplied by 4.
     *           height = 4t(1 - t)
     *   Then to have a max height adjusted according to the range (i.e. greater
     *   the range, larger the max height), it is required to multiplied by
     *   a range adjusted height factor.
     *           height = (range * heighFactor) * 4t (1 - t)
     *
     *   Note: This doesn't consider gravity and calculate actual projectile paths,
     *   but simulate a parabolic equation where the peak height is proportionate
     *   to the travel distance.
     */
    float getHeight(const Feet& pos) const
    {
        auto totalDistance = originPosition.distance(targetPosition);
        auto normalizedTravelDistance = getNormalizedTravelDistance(pos);

        auto maxHeight = totalDistance * ARC_HEIGHT_FACTOR;

        auto arcHeight =
            4.0 * maxHeight * normalizedTravelDistance * (1.0 - normalizedTravelDistance);

        return arcHeight;
    }

    /*
     *   Calculate the visual angle of the projectile based on actual previous
     *   and new pixel position.
     *
     *   Note: This isn't a logical angle (which we don't care anyway).
     */
    float calculateVisualAngle(const Vec2& newPixelPos)
    {
        auto delta = newPixelPos - previousPixelPos;
        previousPixelPos = newPixelPos;

        // Angle from top (i.e. -y direction)
        auto angle = std::atan2(delta.x, -1 * delta.y);
        auto angleDegrees = toDegrees(angle);
        return angleDegrees;
    }

    /*
     *   Deprecated, use calculateVisualAngle
     *
     *   Approach: θ(t)=atan2( A(dx−dy), −[B(dx+dy) − 4Hmax​(1−2t)])
     *   This is derived from getHeight(). And this method is stateless, where
     *   it can derive the angle just by using traveled distance.
     *   But the downside is, it produces sharp logical angles like 5, 175 degrees
     *   instead of more natural around 45 degrees.
     *
     *   Note: This isn't a logical angle (which we don't care anyway).
     */
    float getVisualAngle(const Feet& pos) const
    {
        auto totalDistance = originPosition.distance(targetPosition);
        auto normalizedTravelDistance = getNormalizedTravelDistance(pos);
        auto widthPixelsRatio =
            (float) Constants::TILE_PIXEL_WIDTH / (2 * Constants::FEET_PER_TILE);
        auto heightPixelsRatio =
            (float) Constants::TILE_PIXEL_HEIGHT / (2 * Constants::FEET_PER_TILE);
        auto maxHeight = totalDistance * ARC_HEIGHT_FACTOR;
        auto deltaRange = targetPosition - originPosition;

        auto visualDeltaY = widthPixelsRatio * (deltaRange.x - deltaRange.y);
        auto visualDeltaX = heightPixelsRatio * (deltaRange.x + deltaRange.y) -
                            4 * maxHeight * (1 - 2 * normalizedTravelDistance);

        auto angle = std::atan2(visualDeltaY, -1 * visualDeltaX);
        auto angleDegrees = toDegrees(angle);
        return angleDegrees;
    }

    /*
     *   Return the travel distance normalized to range 0.0 to 1.0.
     */
    float getNormalizedTravelDistance(const Feet& pos) const
    {
        auto totalDistance = originPosition.distance(targetPosition);
        auto travelDistance = originPosition.distance(pos);
        auto normalizedTravelDistance = travelDistance / totalDistance;
        normalizedTravelDistance = std::clamp(normalizedTravelDistance, 0.0f, 1.0f);

        return normalizedTravelDistance;
    }
};
} // namespace core

#endif // CORE_COMPPROJECTILE_H
