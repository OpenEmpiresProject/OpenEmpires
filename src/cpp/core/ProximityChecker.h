#ifndef CORE_PROXIMITYCHECKER_H
#define CORE_PROXIMITYCHECKER_H
#include "StateManager.h"
#include "components/CompBuilding.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"

namespace core
{
class ProximityChecker
{
  public:
    static bool isInProximity(const CompTransform& unitTransform,
                              uint32_t targetEntity,
                              Ref<StateManager> stateMan)
    {
        if (stateMan->hasComponent<CompBuilding>(targetEntity))
        {
            auto& building = stateMan->getComponent<CompBuilding>(targetEntity);
            auto rect = building.getLandInFeetRect();

            auto unitPos = unitTransform.position;
            auto unitRadiusSq = unitTransform.goalRadiusSquared;

            return overlaps(unitPos, unitRadiusSq, rect);
        }
        else if (stateMan->hasComponent<CompResource>(targetEntity))
        {
            auto [transform, resource] =
                stateMan->getComponents<CompTransform, CompResource>(targetEntity);
            auto rect = resource.getLandInFeetRect(transform.position);

            auto unitPos = unitTransform.position;
            auto unitRadiusSq = unitTransform.goalRadiusSquared;

            return overlaps(unitPos, unitRadiusSq, rect);
        }
        else if (stateMan->hasComponent<CompSelectible>(targetEntity))
        {
            auto& targetTransform = stateMan->getComponent<CompTransform>(targetEntity);

            auto unitPos = unitTransform.position;
            auto unitRadiusSq = unitTransform.goalRadiusSquared;

            return overlaps(unitPos, unitRadiusSq, targetTransform.position);
        }
        debug_assert(false, "Unknown entity type for target {}", targetEntity);
    }

    static bool isInProximity(const CompTransform& unitTransform, const Feet& targetpos)
    {
        return unitTransform.position.distanceSquared(targetpos) < unitTransform.goalRadiusSquared;
    }

  private:
    static bool overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect)
    {
        const float xMin = buildingRect.x;
        const float xMax = buildingRect.x + buildingRect.w;
        const float yMin = buildingRect.y;
        const float yMax = buildingRect.y + buildingRect.h;

        const float closestX = std::clamp(unitPos.x, xMin, xMax);
        const float closestY = std::clamp(unitPos.y, yMin, yMax);

        const float dx = unitPos.x - closestX;
        const float dy = unitPos.y - closestY;

        return (dx * dx + dy * dy) <= radiusSq;
    }

    static bool overlaps(const Feet& unitPos, float radiusSq, const Feet& targetPos)
    {
        const float dx = unitPos.x - targetPos.x;
        const float dy = unitPos.y - targetPos.y;

        return (dx * dx + dy * dy) <= radiusSq;
    }
};
} // namespace core

#endif
