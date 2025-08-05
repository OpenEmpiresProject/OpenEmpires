#ifndef PATHFINDERASTAR_H
#define PATHFINDERASTAR_H

#include "PathFinderBase.h"

namespace core
{
class PathFinderAStar : public PathFinderBase
{
  public:
    Path findPath(const TileMap& map, const Feet& start, const Feet& goal) override;
};

} // namespace core
#endif