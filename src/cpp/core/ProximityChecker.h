#ifndef CORE_PROXIMITYCHECKER_H
#define CORE_PROXIMITYCHECKER_H
#include "StateManager.h"
#include "components/CompBuilding.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "utils/Maths.h"

namespace core
{
class ProximityChecker
{
  public:
    static bool isInProximity(const CompTransform& unitTransform,
                              uint32_t targetEntity,
                              Ref<StateManager> stateMan)
    {
        return isInProximity(unitTransform.position, unitTransform.collisionRadius, targetEntity,
                             stateMan);
    }

    static bool isInProximity(const Feet& unitPos,
                              float unitRadius,
                              uint32_t targetEntity,
                              Ref<StateManager> stateMan)
    {
        if (stateMan->hasComponent<CompBuilding>(targetEntity))
        {
            auto& building = stateMan->getComponent<CompBuilding>(targetEntity);

            return building.isOverlapping(unitPos, unitRadius);
        }
        else if (stateMan->hasComponent<CompResource>(targetEntity))
        {
            auto [transform, resource] =
                stateMan->getComponents<CompTransform, CompResource>(targetEntity);
            auto rect = resource.getLandInFeetRect(transform.position);

            return maths::isOverlapping(unitPos, unitRadius, rect);
        }
        else if (stateMan->hasComponent<CompSelectible>(targetEntity))
        {
            auto& targetTransform = stateMan->getComponent<CompTransform>(targetEntity);

            return maths::isOverlapping(unitPos, unitRadius, targetTransform.position);
        }
        debug_assert(false, "Unknown entity type for target {}", targetEntity);
    }

    static bool isInProximity(const CompTransform& unitTransform, const Feet& targetpos)
    {
        return unitTransform.position.distanceSquared(targetpos) <
               (unitTransform.collisionRadius * unitTransform.collisionRadius);
    }

    static bool isInProximity(const Feet& pos, const Feet& targetpos, int goalRadius)
    {
        return pos.distanceSquared(targetpos) < goalRadius;
    }
};
} // namespace core

#endif
