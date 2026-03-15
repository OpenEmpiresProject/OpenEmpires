#include "PathService.h"

#include "PathFinderBase.h"
#include "Player.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "components/CompTransform.h"
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

Feet PathService::getBestAvoidanceDirectionVector(const Feet& currentPos,
                                                  const Feet& preferredVector,
                                                  int collisionRadius,
                                                  uint32_t entity)
{
    float bestScore = std::numeric_limits<float>::lowest();

    auto preferredDir = preferredVector.normalized();
    Feet bestDirection = preferredDir;
    auto& densityGrid = m_stateMan->getDensityGrid();

    auto candidateDirections =
        generateCandidateDirections(preferredDir, NUMBER_OF_CANDIDATE_DIRECTIONS);

    for (auto& candidateDir : candidateDirections)
    {
        // Making side ways be able to win. Otherwise goalScore will be always zero and final
        // score can never be more than zero for side ways.
        float goalScore = (candidateDir.dot(preferredDir) + 1) * 0.5f * GOAL_SCORE_WEIGHT;

        // If the goal score alone less than previous best score, no need to check any other
        // directions.
        if (goalScore <= bestScore)
            return bestDirection;

        Feet predicatedPos =
            currentPos + candidateDir * (collisionRadius * LOOKAHEAD_COLLISION_RADIUS_MULTIPLIER);

        auto density = densityGrid.getDensitySaturated(predicatedPos);

        // Making low density much less important, while high density still matters.
        auto densityScore = density * density * DENSITY_PENALTY_WEIGHT;

        auto separationPenalty = getSeparationPenalty(predicatedPos, entity, collisionRadius);

        auto score = goalScore - densityScore - separationPenalty;

        if (score > bestScore)
        {
            bestScore = score;
            bestDirection = candidateDir;
        }
    }
    return bestDirection;
}

std::vector<core::Feet> PathService::generateCandidateDirections(const Feet& desiredDir,
                                                                 int numSamples)
{
    std::vector<Feet> candidates;

    Feet forward = desiredDir.normalized();
    candidates.push_back(forward); // always include the desired direction

    // angle step for samples around desiredDir
    float angleStep = 360.0f / (float) numSamples;

    for (int i = 1; i <= numSamples; ++i)
    {
        float angle = i * angleStep;
        candidates.push_back(forward.rotated(angle));
    }
    return candidates;
}

float PathService::getSeparationPenalty(const Feet& pos,
                                        uint32_t unitEntity,
                                        int unitCollisionRadius)
{
    Tile center = pos.toTile();
    const Tile searchStartTile = center - Tile(1, 1);
    const Tile searchEndTile = center + Tile(1, 1);

    auto& gameMap = m_stateMan->gameMap();

    float separationPenalty = 0.0f;

    for (int x = searchStartTile.x; x <= searchEndTile.x; x++)
    {
        for (int y = searchStartTile.y; y <= searchEndTile.y; y++)
        {
            Tile gridPos{x, y};
            if (not gameMap.isValidPos(gridPos))
                continue;

            auto& entities = gameMap.getEntities(MapLayerType::UNITS, gridPos);

            for (auto e : entities)
            {
                if (e == entt::null || e == unitEntity)
                    continue;

                auto& otherTransform = m_stateMan->getComponent<CompTransform>(e);
                Feet toOther = otherTransform.position - pos;

                float distSq = toOther.lengthSquared();

                float separationRadius = (unitCollisionRadius + otherTransform.collisionRadius);
                float separationRadiusSq = separationRadius * separationRadius;

                if (distSq < separationRadiusSq)
                {
                    float dist = std::sqrt(distSq);
                    float overlap = separationRadius - dist;

                    separationPenalty += overlap;
                }
            }
        }
    }
    return separationPenalty;
}
