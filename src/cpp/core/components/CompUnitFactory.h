#ifndef COMPUNITFACTORY_H
#define COMPUNITFACTORY_H

#include "Property.h"
#include "utils/Constants.h"

#include <tuple>
#include <vector>

namespace core
{
class CompUnitFactory
{
  public:
    Property<std::vector<uint32_t>> producibleUnitTypes;    // Entity types. TODO: Remove
    Property<std::vector<std::string>> producibleUnitNames; // Entity names. TODO: Remove
    Property<std::unordered_map<char, uint32_t>> producibleUnitShortcuts;
    Property<std::unordered_map<char, std::string>> producibleUnitNamesByShortcuts; // TODO: Remove
    Property<uint32_t> maxQueueSize;
    Property<uint32_t> unitCreationSpeed;

  public:
    std::vector<uint32_t> productionQueue; // Entity types
    float currentUnitProgress = 0;
    bool pausedDueToInsufficientHousing = false;
    bool pausedDueToPopulationLimit = false;

    CompUnitFactory()
    {
        productionQueue.reserve(Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE);
    }

    void onCreate(uint32_t entity)
    {

    }
};

} // namespace core

#endif
