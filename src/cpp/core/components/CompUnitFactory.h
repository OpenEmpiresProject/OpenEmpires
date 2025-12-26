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
    Property<std::vector<uint32_t>> producibleUnitTypes;    // Entity types
    Property<std::vector<std::string>> producibleUnitNames; // Entity names
    Property<std::unordered_map<char, uint32_t>> producibleUnitShortcuts;
    Property<std::unordered_map<char, std::string>> producibleUnitNamesByShortcuts;
    Property<uint32_t> maxQueueSize;
    Property<uint32_t> unitCreationSpeed;

  public:
    static constexpr auto properties()
    {
        return std::tuple{PropertyDesc<&CompUnitFactory::maxQueueSize>{"UnitFactory", "max_queue_size"},
                          PropertyDesc<&CompUnitFactory::unitCreationSpeed>{"UnitFactory", "unit_creation_speed"}};
    }

  public:
    std::vector<uint32_t> productionQueue; // Entity types
    float currentUnitProgress = 0;
    bool pausedDueToInsufficientHousing = false;
    bool pausedDueToPopulationLimit = false;

    CompUnitFactory()
    {
        productionQueue.reserve(Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE);
    }
};

} // namespace core

#endif
