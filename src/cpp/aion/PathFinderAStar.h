#ifndef PATHFINDERASTAR_H
#define PATHFINDERASTAR_H

#include "PathFinderBase.h"

namespace aion
{
class PathFinderAStar : public PathFinderBase
{
  public:
    Path findPath(const StaticEntityMap& map, const Vec2d& start, const Vec2d& goal) override;
};

} // namespace aion
#endif