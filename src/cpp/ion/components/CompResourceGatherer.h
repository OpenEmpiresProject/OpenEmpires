#ifndef COMPRESOURCEGATHERER_H
#define COMPRESOURCEGATHERER_H

#include "Resource.h"
#include "utils/Constants.h"

#include <unordered_map>

namespace ion
{
struct CompResourceGatherer
{
    // TODO: Can this be static?
    // Gathering action per resource type
    std::unordered_map<uint8_t, uint32_t> gatheringAction;
    uint32_t capacity = 0;
    uint32_t gatheredAmount = 0;

    bool canGather(uint8_t resourceType)
    {
        return gatheringAction.contains(resourceType);
    }

    uint32_t getGatheringAction(uint8_t resourceType)
    {
        return gatheringAction[resourceType];
    }
};
} // namespace ion

#endif