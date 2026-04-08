#ifndef MATHS_H
#define MATHS_H

#include "Feet.h"
#include "Rect.h"

#include <algorithm>

namespace core
{
namespace maths
{
/*
 *   Check whether the rectangle overlaps with the circle
 */
static inline bool isOverlapping(const Feet& point, float radius, const Rect<float>& rect)
{
    const float xMin = rect.x;
    const float xMax = rect.x + rect.w;
    const float yMin = rect.y;
    const float yMax = rect.y + rect.h;

    const float closestX = std::clamp(point.x, xMin, xMax);
    const float closestY = std::clamp(point.y, yMin, yMax);

    const float dx = point.x - closestX;
    const float dy = point.y - closestY;

    return (dx * dx + dy * dy) <= (radius * radius);
}

/*
 *   Check whether the point overlaps with the circle
 */
static inline bool isOverlapping(const Feet& point, float radius, const Feet& pointToCheck)
{
    const float dx = point.x - pointToCheck.x;
    const float dy = point.y - pointToCheck.y;

    return (dx * dx + dy * dy) <= (radius * radius);
}

/*
 *   Check whether the point overlaps with the rectangle with a radius.
 *   Essentially rounded-cornered square overlaps with the point.
 */
static inline bool isOverlapping(const Rect<float>& rect,
                                 float rectRadius,
                                 const Feet& pointToCheck)
{
    // Expanded rectRadius rectangle
    float left = rect.x - rectRadius;
    float right = rect.x + rect.w + rectRadius;
    float top = rect.y - rectRadius;
    float bottom = rect.y + rect.h + rectRadius;

    // Quick reject
    if (pointToCheck.x < left || pointToCheck.x > right || pointToCheck.y < top ||
        pointToCheck.y > bottom)
        return false;

    // Axis-aligned inner rectangle (straight LOS zones)
    // If point is horizontally aligned with building
    if (pointToCheck.x >= rect.x && pointToCheck.x <= rect.x + rect.w)
        return true;

    // If point is vertically aligned with building
    if (pointToCheck.y >= rect.y && pointToCheck.y <= rect.y + rect.h)
        return true;

    // Corner checks: quarter circles of radius LOS
    auto sq = [](float v) { return v * v; };

    // top-left
    float dx = pointToCheck.x - rect.x;
    float dy = pointToCheck.y - rect.y;
    if (dx < 0 && dy < 0 && sq(dx) + sq(dy) <= rectRadius * rectRadius)
        return true;

    // top-right
    dx = pointToCheck.x - (rect.x + rect.w);
    dy = pointToCheck.y - rect.y;
    if (dx > 0 && dy < 0 && sq(dx) + sq(dy) <= rectRadius * rectRadius)
        return true;

    // bottom-left
    dx = pointToCheck.x - rect.x;
    dy = pointToCheck.y - (rect.y + rect.h);
    if (dx < 0 && dy > 0 && sq(dx) + sq(dy) <= rectRadius * rectRadius)
        return true;

    // bottom-right
    dx = pointToCheck.x - (rect.x + rect.w);
    dy = pointToCheck.y - (rect.y + rect.h);
    if (dx > 0 && dy > 0 && sq(dx) + sq(dy) <= rectRadius * rectRadius)
        return true;

    return false;
}

} // namespace maths
} // namespace core

#endif // CORE_UTILS_H
