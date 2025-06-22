#ifndef PATHFINDERASTAR_H
#define PATHFINDERASTAR_H

#include "PathFinderBase.h"

namespace ion
{
class PathFinderAStar : public PathFinderBase
{
  public:
    Path findPath(const TileMap& map, const Feet& start, const Feet& goal) override;
};

} // namespace ion
#endif