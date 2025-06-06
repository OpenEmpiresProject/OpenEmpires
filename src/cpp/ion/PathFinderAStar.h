#ifndef PATHFINDERASTAR_H
#define PATHFINDERASTAR_H

#include "PathFinderBase.h"

namespace ion
{
class PathFinderAStar : public PathFinderBase
{
  public:
    Path findPath(const GridMap& map, const Vec2d& start, const Vec2d& goal) override;
};

} // namespace ion
#endif