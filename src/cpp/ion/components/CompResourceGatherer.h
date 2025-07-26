#ifndef COMPRESOURCEGATHERER_H
#define COMPRESOURCEGATHERER_H

#include "Property.h"
#include "Resource.h"
#include "utils/Constants.h"
#include "utils/Types.h"

#include <unordered_map>

namespace ion
{
class CompResourceGatherer
{
  public:
    // Gathering action per resource type
    static std::unordered_map<uint8_t, UnitAction> gatheringActions;
    // Resource carrying action per resource type
    static std::unordered_map<uint8_t, UnitAction> carryingActions;

    Property<uint32_t> capacity;
    Property<uint32_t> gatherSpeed;

    uint32_t gatheredAmount = 0;

    bool canGather(uint8_t resourceType)
    {
        return gatheringActions.contains(resourceType);
    }

    UnitAction getGatheringAction(uint8_t resourceType)
    {
        return gatheringActions[resourceType];
    }

    UnitAction getCarryingAction(uint8_t resourceType)
    {
        return carryingActions[resourceType];
    }
};
} // namespace ion

#endif