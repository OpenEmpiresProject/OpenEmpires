#ifndef PATHFINDERBASE_H
#define PATHFINDERBASE_H

#include "StaticEntityMap.h"
#include "vec2d.h"

namespace aion
{
using Path = std::vector<Vec2d>;

class PathFinderBase
{
  public:
    virtual Path findPath(const StaticEntityMap& map, const Vec2d& start, const Vec2d& goal) = 0;
};

} // namespace aion

#endif