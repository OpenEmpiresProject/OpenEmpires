#ifndef PATHFINDERBASE_H
#define PATHFINDERBASE_H

#include "Feet.h"
#include "TileMap.h"

namespace core
{
using Path = std::vector<Feet>;

class PathFinderBase
{
  public:
    virtual Path findPath(const TileMap& map, const Feet& start, const Feet& goal) = 0;
};

} // namespace core

#endif