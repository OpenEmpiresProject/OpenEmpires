#ifndef PATHFINDERBASE_H
#define PATHFINDERBASE_H

#include "Feet.h"
#include "PassabilityMap.h"
#include "Player.h"

namespace core
{
using Path = std::vector<Feet>;

class PathFinderBase
{
  public:
    virtual Path findPath(const PassabilityMap& map,
                          Ref<Player> player,
                          const Feet& start,
                          const Feet& goal) = 0;
};

} // namespace core

#endif