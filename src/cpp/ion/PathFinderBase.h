#ifndef PATHFINDERBASE_H
#define PATHFINDERBASE_H

#include "GridMap.h"
#include "vec2d.h"

namespace ion
{
using Path = std::vector<Vec2d>;

class PathFinderBase
{
  public:
    virtual Path findPath(const GridMap& map, const Vec2d& start, const Vec2d& goal) = 0;
};

} // namespace ion

#endif