#include "PathService.h"

#include "LineUnitFormation.h"
#include "PathFinderBase.h"
#include "Player.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "components/CompPlayer.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "logging/Logger.h"

#include <optional>

using namespace core;

PathService::PathService()
    : m_maxDirectPathDistanctInFeetSquared(
          std::pow(Constants::MAX_DIRECT_PATH_DISTANCE_IN_TILES * Constants::FEET_PER_TILE, 2))
{
}

/*
 *  Approach: If the target is closer than MAX_DIRECT_PATH_DISTANCE_IN_TILES
 *  and there is a clear line of sight, we skip pathfinding and return a direct path.
 *  Then we use the pathfinder to find a path. After we get the path, we refine it by removing
 *  any intermediate waypoints that are directly visible from the last kept waypoint.
 */
Path PathService::findPath(const Feet& from, const Feet& to, Ref<Player> player)
{
    auto& passabilityMap = m_stateMan->getPassabilityMap();
    if (not passabilityMap.isPassableFor(to.toTile(), player->getId()))
    {
        return Path(); // Return empty path if destination is not passable
    }
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

    // Remove the starting position from the waypoints, as it's not needed for movement
    // but only needed for path refinement.
    path.waypoints.erase(path.waypoints.begin());
    return path;
}

core::Path PathService::findPath(const Feet& from,
                                 const Feet& to,
                                 Ref<Player> player,
                                 int currentTick)
{
    auto it = m_pathCache.find(computePathCacheKey(from, to, player->getId()));
    if (it != m_pathCache.end() and it->second.lastAccessedTick)
    {
        auto ticksElapsed = currentTick - it->second.lastAccessedTick;
        if (ticksElapsed < PATH_CACHE_TTL_IN_TICKS)
        {
            it->second.lastAccessedTick = currentTick;
            return it->second.path;
        }
    }
    auto path = findPath(from, to, player);
    m_pathCache.insert({computePathCacheKey(from, to, player->getId()), {path, currentTick}});

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
    auto fromTile = from.toTile();
    auto toTile = to.toTile();

    if (fromTile == toTile)
        return true;

    auto& passabilityMap = m_stateMan->getPassabilityMap();
    float distance = from.distance(to);
    auto stepGranularity = distance < (Constants::FEET_PER_TILE * 2) ? 0.1 : 0.25f;
    auto numSteps = static_cast<int>(distance / (Constants::FEET_PER_TILE * stepGranularity));

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

float PathService::getSeparationPenaltyScore(const Feet& pos,
                                             uint32_t unitEntity,
                                             int unitCollisionRadius,
                                             const std::list<uint32_t> neighbors)
{
    Tile center = pos.toTile();
    const Tile searchStartTile = center - Tile(1, 1);
    const Tile searchEndTile = center + Tile(1, 1);

    auto& gameMap = m_stateMan->gameMap();

    float separationPenalty = 0.0f;
    auto& unitComp = m_stateMan->getComponent<CompUnit>(unitEntity);
    auto isInFormation = unitComp.formationSlot.isValid();
    auto formation = unitComp.formationSlot.getFormation();
    bool foundCollisionWithinFormation = false;

    for (auto e : neighbors)
    {
        auto [otherUnit, otherTransform] = m_stateMan->getComponents<CompUnit, CompTransform>(e);
        Feet toOther = otherTransform.position - pos;

        float distSq = toOther.lengthSquared();

        float separationRadius = (unitCollisionRadius + otherTransform.collisionRadius);
        float separationRadiusSq = separationRadius * separationRadius;

        if (distSq < separationRadiusSq)
        {
            float dist = std::sqrt(distSq);
            float overlap = separationRadius - dist;

            if (otherUnit.formationSlot.isValid() and
                formation == otherUnit.formationSlot.getFormation())
            {
                foundCollisionWithinFormation = true;
            }
            else [[likely]]
            {
                separationPenalty += overlap;
            }
        }
    }

    if (foundCollisionWithinFormation)
        separationPenalty += 0.25f;
    return separationPenalty;
}

core::Ref<core::BaseUnitFormation> PathService::createFormation(
    const std::vector<uint32_t>& unitEntityIds, UnitFormationType type, const Feet& target)
{
    if (type == UnitFormationType::LINE_FORMATION)
    {
        auto lineFormation = CreateRef<LineUnitFormation>();
        lineFormation->createFormation(unitEntityIds, target);

        for (auto& slot : lineFormation->getSlots())
        {
            auto& unit = m_stateMan->getComponent<CompUnit>(slot.getEntityId());
            unit.formationSlot = slot;
        }

        return std::move(lineFormation);
    }
    // Add other formation types factories here

    return nullptr;
}

void PathService::deleteFormation(Ref<BaseUnitFormation> formation)
{
    for (auto& slot : formation->getSlots())
    {
        auto entity = slot.getEntityId();
        if (entity != entt::null)
        {
            // TODO: might be able to avoid this by making slots Refs/pointers
            auto& compUnit = m_stateMan->getComponent<CompUnit>(entity);
            compUnit.formationSlot = FormationSlot(entt::null, nullptr); // Invalidate slot
        }
    }
    formation->deleteFormation();
}

core::Feet PathService::getBestAvoidanceDirectionVector(const Feet& currentPos,
                                                        const Feet& preferredVector,
                                                        int collisionRadius,
                                                        int speed,
                                                        std::optional<float> lookAheadDurationSecs,
                                                        uint32_t entity,
                                                        AvoidnaceQuality quality,
                                                        const Target& target)
{
    float bestScore = std::numeric_limits<float>::lowest();

    auto preferredDir = preferredVector.normalized();
    Feet bestDirection = preferredDir;
    auto& densityGrid = m_stateMan->getDensityGrid();
    auto lookAheadTime = lookAheadDurationSecs.value_or(DEFAULT_LOOK_AHEAD_DURATION_IN_SECONDS);

    auto [unitComp, transform] = m_stateMan->getComponents<CompUnit, CompTransform>(entity);
    auto formation = unitComp.formationSlot.getFormation();
    auto previousDir = transform.getVelocityVector().normalized();

    auto candidateDirections =
        generateCandidateDirections(preferredDir, NUMBER_OF_CANDIDATE_DIRECTIONS);

    // Cache neighbors if required to avoid repeating this for each candidate direction.
    // Optimal solution would be to use the predicted pos, but this is faster to
    // first read all neighbors and then collision check.
    //
    std::list<uint32_t> neighbors = getNeighbors(currentPos, entity, target.entity);

    for (auto& candidateDir : candidateDirections)
    {
        // Making side ways be able to win. Otherwise goalScore will be always zero and final
        // score can never be more than zero for side ways.
        float goalScore = (candidateDir.dot(preferredDir) + 1) * 0.5f * GOAL_SCORE_WEIGHT;

        Feet predicatedPos = currentPos + candidateDir * (lookAheadTime * speed);
        float avoidanceScore = 0.0f;

        if (quality == AvoidnaceQuality::HIGH)
        {
            avoidanceScore = getGeometricAvoidanceScore(currentPos, candidateDir, speed,
                                                        collisionRadius, lookAheadTime, neighbors);
        }
        else if (quality == AvoidnaceQuality::MEDIUM)
        {
            auto density = densityGrid.getDensitySaturated(predicatedPos);
            auto densityPenaltyWeight = DENSITY_PENALTY_WEIGHT;
            avoidanceScore = density * density * densityPenaltyWeight;
        }

        auto separationPenaltyScore =
            getSeparationPenaltyScore(predicatedPos, entity, collisionRadius, neighbors);

        float alignmentWithPrev = candidateDir.dot(previousDir);
        auto inertiaScore = INERTIA_TO_CHANGE_DIRECTION * alignmentWithPrev;

        spam("{} Cand dir: {}, Scores; goal {}, avoidance {}, separation {}, inertia {}", entity,
             candidateDir.toString(), goalScore, avoidanceScore, separationPenaltyScore,
             inertiaScore);

        auto score = goalScore - avoidanceScore - separationPenaltyScore + inertiaScore;

        if (score > bestScore)
        {
            bestScore = score;
            bestDirection = candidateDir;
        }
    }
    spam("{}, preferred dir {}, best dir {}", entity, preferredDir.toString(),
         bestDirection.toString());

    return bestDirection.normalized();
}

float PathService::getGeometricAvoidanceScore(const Feet& pos,
                                              const Feet& forward,
                                              int speed,
                                              int collisionRadius,
                                              float lookAheadDurationSecs,
                                              const std::list<uint32_t> neighbors)
{
    Feet dir = forward.normalized();

    for (auto e : neighbors)
    {
        auto& otherTransform = m_stateMan->getComponent<CompTransform>(e);

        // TODO: This doesn't work since there is no proper velocity vector concept yet
        // in the transform.
        Feet otherVel = otherTransform.getVelocityVector();
        Feet otherForward = otherVel.normalized();

        int otherSpeed = static_cast<int>(otherTransform.speed);
        otherSpeed = 0; // TEMP: We need proper velocity vector
        int otherCollisionRadius = otherTransform.collisionRadius;

        if (willCollide(pos, dir, speed, collisionRadius, otherTransform.position, otherForward,
                        otherSpeed, lookAheadDurationSecs, otherCollisionRadius))
        {
            return 1.0f;
        }
    }
    return 0.0f;
}

/**
 * @brief Predicts whether this unit will collide with another moving unit within a given time
 * horizon.
 *
 * This function performs continuous collision detection using relative motion.
 * It determines whether two circular units will overlap at any point in the future
 * within the specified look-ahead duration.
 *
 * ---
 * Core Idea:
 * Instead of simulating movement step-by-step, both units are reduced to a single
 * relative motion problem.
 * The function then computes the point of closest approach over time and checks
 * whether the distance between the units at that point is less than or equal to
 * the sum of their radii.
 */
bool PathService::willCollide(const Feet& pos,
                              const Feet& forward,
                              int speed,
                              int collisionRadius,
                              const Feet& otherPos,
                              const Feet& otherForward,
                              int otherSpeed,
                              int lookAheadDuration,
                              int otherCollisionRadius)
{
    Feet d = pos - otherPos;

    float R = (float) (collisionRadius + otherCollisionRadius);
    float R2 = R * R;

    // too far to ever collide
    float maxReach = (speed + otherSpeed) * (float) lookAheadDuration + R;
    if (d.dot(d) > maxReach * maxReach)
        return false;

    Feet v1 = forward * (float) speed;
    Feet v2 = otherForward * (float) otherSpeed;
    Feet v = v1 - v2;

    float vv = v.dot(v);

    // no relative motion, no future collision
    if (vv < 1e-6f)
        return false;

    float dv = d.dot(v);

    // moving away, no future collision
    if (dv > 0)
        return false;

    float t = -dv / vv;
    t = std::max(0.0f, std::min((float) lookAheadDuration, t));

    Feet closest = d + v * t;

    return closest.dot(closest) <= R2;
}

uint64_t PathService::computePathCacheKey(const Feet& from, const Feet& to, uint32_t playerId) const
{
    auto fromTile = from.toTile();
    auto toTile = to.toTile();

    uint64_t key = 0;

    key |= (uint64_t(playerId) & 0x7F) << 52;     // 7 bits
    key |= (uint64_t(fromTile.x) & 0x1FFF) << 39; // 13 bits
    key |= (uint64_t(fromTile.y) & 0x1FFF) << 26;
    key |= (uint64_t(toTile.x) & 0x1FFF) << 13;
    key |= (uint64_t(toTile.y) & 0x1FFF);

    return key;
}

std::list<uint32_t> PathService::getNeighbors(const Feet& pos,
                                              uint32_t entity,
                                              std::optional<uint32_t> excludeEntity) const
{
    std::list<uint32_t> neighbors;

    Tile center = pos.toTile();
    Tile start = center - 1;
    Tile end = center + 1;
    uint32_t exclude = excludeEntity.value_or(entt::null);

    auto& gameMap = m_stateMan->gameMap();

    for (int x = start.x; x <= end.x; ++x)
    {
        for (int y = start.y; y <= end.y; ++y)
        {
            Tile t{x, y};
            if (!gameMap.isValidPos(t))
                continue;

            const auto& entities = gameMap.getEntities(MapLayerType::UNITS, t);
            for (auto e : entities)
            {
                if (e == entt::null or e == entity or e == exclude)
                    continue;

                neighbors.push_back(e);
            }
        }
    }
    return neighbors;
}

// Note: Anchor is left-top corner of the rect.
Feet getClosestPosToRect(const Feet& fromPos, const Rect<float>& rect)
{
    const float xMin = rect.x;
    const float xMax = rect.x + rect.w;
    const float yMin = rect.y;
    const float yMax = rect.y + rect.h;

    const float closestX = std::clamp(fromPos.x, xMin, xMax);
    const float closestY = std::clamp(fromPos.y, yMin, yMax);

    return {closestX, closestY};
}

Tile getClosestTile(const Feet& fromPos, const std::vector<Tile>& tiles)
{
    Tile closestTile = Tile::null;
    float closestDistSq = std::numeric_limits<float>::max();

    for (auto& tile : tiles)
    {
        auto distanceSq = fromPos.distanceSquared(tile.centerInFeet());
        if (distanceSq < closestDistSq)
        {
            closestTile = tile;
            closestDistSq = distanceSq;
        }
    }
    return closestTile;
}

Tile getClosestBoundaryTile(const Feet& fromPos, const Tile& centerTile)
{
    Feet centerInFeet = centerTile.centerInFeet();
    Rect<float> rect;
    rect.x = centerInFeet.x - Constants::FEET_PER_TILE;
    rect.y = centerInFeet.y - Constants::FEET_PER_TILE;
    rect.w = rect.h = Constants::FEET_PER_TILE * 2;

    return getClosestPosToRect(fromPos, rect).toTile();
}

int getTileHash(const Tile& tile)
{
    return tile.x * 10000 + tile.y;
}

/*
*   Approach:
*   1. Find the closest boundary (immediate out layer) tile of the land area
        from the position
*   2. Perform BFS from that tile to find the closest vacant tile
*/
core::Feet PathService::findClosestVacantPosAroundLand(uint32_t forUnit,
                                                       const Feet& fromPos,
                                                       const LandArea& land) const
{
    debug_assert(land.tiles.size() > 0,
                 "Land area must be non-empty to find the closest vacant pos");

    auto [compPlayer, compTransform] =
        m_stateMan->getComponents<CompPlayer, CompTransform>(forUnit);
    auto& passabilityMap = m_stateMan->getPassabilityMap();
    auto playerId = compPlayer.player->getId();

    std::unordered_set<int> alreadyWalked;

    constexpr std::array<Tile, 8> surroundingOffsets = {Tile(-1, 0), Tile(0, -1),  Tile(1, 0),
                                                        Tile(0, 1),  Tile(-1, -1), Tile(1, -1),
                                                        Tile(1, 1),  Tile(-1, 1)};

    for (auto& tile : land.tiles)
    {
        alreadyWalked.insert(getTileHash(tile));
    }

    int closestDistanceSq = std::numeric_limits<int>::max();
    Tile closestTile = Tile::null;

    // TODO: Could optimize this to a void the brute force
    for (auto& tile : land.tiles)
    {
        for (auto& offset : surroundingOffsets)
        {
            Tile surrounding = tile + offset;
            auto tileHash = getTileHash(surrounding);
            if (not alreadyWalked.contains(tileHash))
            {
                int distanceSq = fromPos.distanceSquared(surrounding.centerInFeet());
                alreadyWalked.insert(tileHash);

                if (distanceSq < closestDistanceSq and
                    passabilityMap.isPassableFor(surrounding, playerId))
                {
                    closestDistanceSq = distanceSq;
                    closestTile = surrounding;
                }
            }
        }
    }

    if (closestTile == Tile::null)
    {
        // fall back
        return fromPos;
    }

    /*
     *   Now we have the closest surrounding tile to the fromPos. But we need
     *   to find the closest edge of the land area.
     *   Iterate through all land tiles and find the closest land tile, then
     *   calculate the Feet position just outside (45% towards the land) that
     *   land tile.
     */

    int closestTileDistanceSq = std::numeric_limits<int>::max();
    Tile closestLandTile = Tile::null;

    for (auto& tile : land.tiles)
    {
        int distanceSq = closestTile.distanceSquared(tile);
        if (distanceSq < closestTileDistanceSq)
        {
            closestLandTile = tile;
            closestTileDistanceSq = distanceSq;
        }
    }

    Feet directionToLandTile = closestLandTile.centerInFeet() - closestTile.centerInFeet();
    return closestTile.centerInFeet() + (directionToLandTile * 0.45f);
}