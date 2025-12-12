#ifndef PATHFINDERASTAR_H
#define PATHFINDERASTAR_H

#include "PathFinderBase.h"

namespace core
{
class PathFinderAStar : public PathFinderBase
{
  public:
    Path findPath(const PassabilityMap& map,
                  Ref<Player> player,
                  const Feet& start,
                  const Feet& goal) override;
};

} // namespace core
#endif