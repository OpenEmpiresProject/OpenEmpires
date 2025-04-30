#ifndef PATHFINDERASTAR_H
#define PATHFINDERASTAR_H

#include "PathFinderBase.h"

#include <unordered_map>

namespace aion
{
class PathFinderAStar : public PathFinderBase
{
  public:
    Path findPath(const StaticEntityMap& map, const Vec2d& start, const Vec2d& goal) override;

  private:
    int heuristic(const Vec2d& a, const Vec2d& b);
    bool isWalkable(const StaticEntityMap& map, const Vec2d& pos);
    std::vector<Vec2d> getNeighbors(const Vec2d& pos);
    Path reconstructPath(const std::unordered_map<Vec2d, Vec2d>& cameFrom, Vec2d current);
};

} // namespace aion
#endif