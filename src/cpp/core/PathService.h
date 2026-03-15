#ifndef CORE_PATHSERVICE_H
#define CORE_PATHSERVICE_H

#include "Feet.h"
#include "Path.h"
#include "StateManager.h"

namespace core
{
class PathFinderBase;
class Player;
class PathService
{
  public:
    PathService();
    ~PathService();

    Path findPath(const Feet& from, const Feet& to, Ref<Player> player);
    void refinePath(Path& path, Ref<Player> player) const;
    bool canTraverseDirectly(const Feet& from, const Feet& to, Ref<Player> player) const;
    Feet getBestAvoidanceDirectionVector(const Feet& currentPos,
                                         const Feet& preferredVector,
                                         int collisionRadius,
                                         uint32_t entity);
    float getSeparationPenalty(const Feet& pos, uint32_t unitEntity, int unitCollisionRadius);
    std::vector<Feet> generateCandidateDirections(const Feet& desiredDir, int numSamples);

    const float GOAL_SCORE_WEIGHT = 1.0f;
    const float DENSITY_PENALTY_WEIGHT = 4.0f;
    const int NUMBER_OF_CANDIDATE_DIRECTIONS = 16;
    const int LOOKAHEAD_COLLISION_RADIUS_MULTIPLIER = 2;

  private:
    Ref<PathFinderBase> m_pathFinder;
    LazyServiceRef<StateManager> m_stateMan;
    const int m_maxDirectPathDistanctInFeetSquared;
};
} // namespace core

#endif // CORE_PATHSERVICE_H
