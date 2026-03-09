#include "PathService.h"

#include "PathFinderBase.h"
#include "Player.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "logging/Logger.h"

using namespace core;

PathService::PathService()
    : m_maxDirectPathDistanctInFeetSquared(
          std::pow(Constants::MAX_DIRECT_PATH_DISTANCE_IN_TILES * Constants::FEET_PER_TILE, 2))
{
    // constructor
}

PathService::~PathService()
{
    // destructor
}

/*
 *  Approach: If the target is closer than MAX_DIRECT_PATH_DISTANCE_IN_TILES
 *  and there is a clear line of sight, we skip pathfinding and return a direct path.
 *  Then we use the pathfinder to find a path. After we get the path, we refine it by removing
 *  any intermediate waypoints that are directly visible from the last kept waypoint.
 */
Path PathService::findPath(const Feet& from, const Feet& to, Ref<Player> player)
{
    if (from.distanceSquared(to) < (float) m_maxDirectPathDistanctInFeetSquared)
    {
        if (canTraverseDirectly(from, to, player))
        {
            spam("Direct path from {} to {} is clear, skipping pathfinding", from.toString(),
                 to.toString());
            return Path({to});
        }
    }

    spam("Direct path from {} to {} is NOT clear, using pathfinding", from.toString(),
         to.toString());

    if (m_pathFinder == nullptr)
        m_pathFinder = m_stateMan->getPathFinder();

    auto waypoints = m_pathFinder->findPath(m_stateMan->getPassabilityMap(), player, from, to);
    Path path(waypoints);
    path.waypoints.insert(path.waypoints.begin(), from);
    path.waypoints.push_back(to);

    refinePath(path, player);

    return path;
}

void PathService::refinePath(Path& path, Ref<Player> player) const
{
    if (path.waypoints.size() < 3)
    {
        spdlog::debug("Either path is empty or has less than 3 points. Nothing to refine");
        return;
    }

    spdlog::debug("Refining path with {} waypoints", path.waypoints.size());

    auto prevIt = path.waypoints.begin();
    auto it = std::next(prevIt);

    while (std::next(it) != path.waypoints.end())
    {
        auto nextIt = std::next(it);

        if (canTraverseDirectly(*prevIt, *nextIt, player))
        {
            spdlog::debug("Removing redundant waypoint {}", it->toString());
            it = path.waypoints.erase(it);
        }
        else
        {
            prevIt = it;
            ++it;
        }
    }

    spam("Refine complete, resulting waypoints: {}", path.waypoints.size());
}

bool PathService::canTraverseDirectly(const Feet& from, const Feet& to, Ref<Player> player) const
{
    auto& passabilityMap = m_stateMan->getPassabilityMap();
    float distance = from.distance(to);
    auto numSteps =
        static_cast<int>(distance / (Constants::FEET_PER_TILE * 0.25f)); // Sample every 1/4 tile

    if (numSteps <= 0)
        return false;

    Feet step = (to - from) / static_cast<float>(numSteps);

    for (int i = 0; i <= numSteps; ++i)
    {
        Feet point = from + step * static_cast<float>(i);
        Tile tile = point.toTile();

        if (not passabilityMap.isPassableFor(tile, player->getId()))
            return false;
    }
    return true; // Clear line
}
