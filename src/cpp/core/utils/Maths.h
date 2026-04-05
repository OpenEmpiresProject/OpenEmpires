#ifndef MATHS_H
#define MATHS_H

#include "Feet.h"
#include "Rect.h"

#include <algorithm>

namespace core
{
namespace maths
{
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

static inline bool isOverlapping(const Feet& point, float radius, const Feet& pointToCheck)
{
    const float dx = point.x - pointToCheck.x;
    const float dy = point.y - pointToCheck.y;

    return (dx * dx + dy * dy) <= (radius * radius);
}

} // namespace maths
} // namespace core

#endif // CORE_UTILS_H
