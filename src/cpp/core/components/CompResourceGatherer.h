#ifndef COMPRESOURCEGATHERER_H
#define COMPRESOURCEGATHERER_H

#include "InGameResource.h"
#include "Property.h"
#include "utils/Constants.h"
#include "utils/Types.h"

#include <unordered_map>

namespace core
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

  public:
    static constexpr auto properties()
    {
        return std::tuple{
            PropertyDesc<&CompResourceGatherer::capacity>{"Gatherer", "resource_capacity"},
            PropertyDesc<&CompResourceGatherer::gatherSpeed>{"Gatherer", "gather_speed"}};
    }

  public:
    uint32_t gatheredAmount = 0;

    static bool canGather(uint8_t resourceType)
    {
        return gatheringActions.contains(resourceType);
    }

    static UnitAction getGatheringAction(uint8_t resourceType)
    {
        return gatheringActions[resourceType];
    }

    static UnitAction getCarryingAction(uint8_t resourceType)
    {
        return carryingActions[resourceType];
    }
};
} // namespace core

#endif