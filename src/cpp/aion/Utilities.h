#ifndef UTILITIES_H
#define UTILITIES_H

#include "Vec2d.h"

namespace aion
{
class Utilities
{
public:
    static bool isInsideRhombus(const Vec2d& point, const Vec2d& center, int halfWidth, int halfHeight)
    {
        double dx = std::abs(point.x - center.x);
        double dy = std::abs(point.y - center.y);
        return (dx / halfWidth + dy / halfHeight) <= 1.0;
    }

    Utilities() = delete;
    ~Utilities() = delete;
};

    
} // namespace aion


#endif