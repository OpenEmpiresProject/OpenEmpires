#ifndef CORE_PATHSERVICE_H
#define CORE_PATHSERVICE_H

#include "BaseUnitFormation.h"
#include "Feet.h"
#include "Path.h"
#include "StateManager.h"
#include "Target.h"

namespace core
{
class PathFinderBase;
class Player;

struct PathCacheEntry
{
    Path path;
    int lastAccessedTick;
};

enum class AvoidnaceQuality
{
    MEDIUM,
    HIGH,
};

class PathService
{
  public:
    PathService();
    ~PathService() = default;

    // Path finding related
    Path findPath(const Feet& from, const Feet& to, Ref<Player> player, int currentTick);
    Path findPath(const Feet& from, const Feet& to, Ref<Player> player);
    void refinePath(Path& path, Ref<Player> player) const;
    bool canTraverseDirectly(const Feet& from, const Feet& to, Ref<Player> player) const;

    // Collision avoidance related
    Feet getBestAvoidanceDirectionVector(const Feet& currentPos,
                                         const Feet& preferredVector,
                                         int collisionRadius,
                                         int speed,
                                         std::optional<float> lookAheadDurationSecs,
                                         uint32_t entity,
                                         AvoidnaceQuality quality,
                                         const Target& target);

    // Formation related
    Ref<BaseUnitFormation> createFormation(const std::vector<uint32_t>& unitEntityIds,
                                           UnitFormationType type,
                                           const Feet& target);
    void deleteFormation(Ref<BaseUnitFormation> formation);

    // Occupancy
    Feet findClosestVacantPosAroundLand(uint32_t forUnit,
                                        const Feet& fromPos,
                                        const LandArea& land) const;

    // Constants
    const float GOAL_SCORE_WEIGHT = 1.0f;
    const float DENSITY_PENALTY_WEIGHT = 4.0f;
    const int NUMBER_OF_CANDIDATE_DIRECTIONS = 8;
    const int DEFAULT_LOOK_AHEAD_DURATION_IN_SECONDS = 1;
    const int PATH_CACHE_TTL_IN_TICKS = 300;
    const float INERTIA_TO_CHANGE_DIRECTION = 0.3f;

  protected:
    float getSeparationPenaltyScore(const Feet& pos,
                                    uint32_t unitEntity,
                                    int unitCollisionRadius,
                                    const std::list<uint32_t> neighbors);
    std::vector<Feet> generateCandidateDirections(const Feet& desiredDir, int numSamples);
    float getGeometricAvoidanceScore(const Feet& pos,
                                     const Feet& forward,
                                     int speed,
                                     int collisionRadius,
                                     float lookAheadDurationSecs,
                                     const std::list<uint32_t> neighbors);
    std::list<uint32_t> getNeighbors(const Feet& pos,
                                     uint32_t entity,
                                     std::optional<uint32_t> excludeEntity) const;
    bool willCollide(const Feet& pos,
                     const Feet& forward,
                     int speed,
                     int collisionRadius,
                     const Feet& otherPos,
                     const Feet& otherForward,
                     int otherSpeed,
                     int lookAheadDuration,
                     int otherCollisionRadius);
    uint64_t computePathCacheKey(const Feet& from, const Feet& to, uint32_t playerId) const;

  private:
    Ref<PathFinderBase> m_pathFinder;
    LazyServiceRef<StateManager> m_stateMan;
    const int m_maxDirectPathDistanctInFeetSquared;
    std::unordered_map<uint64_t, PathCacheEntry> m_pathCache;
};
} // namespace core

#endif // CORE_PATHSERVICE_H
