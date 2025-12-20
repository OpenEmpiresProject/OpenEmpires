#ifndef CORE_UTILS_H
#define CORE_UTILS_H
#include "Tile.h"
#include "Types.h"

namespace core
{
class Utils
{
  public:
    Utils() = delete;

    static void calculateConnectedBuildingsPath(
        const Tile& start, const Tile& end, std::list<TilePosWithOrientation>& connectedBuildings);
};
} // namespace core

#endif // CORE_UTILS_H
