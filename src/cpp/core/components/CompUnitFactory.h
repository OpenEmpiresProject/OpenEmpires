#ifndef COMPUNITFACTORY_H
#define COMPUNITFACTORY_H

#include "Property.h"
#include "utils/Constants.h"

#include <vector>

namespace core
{
class CompUnitFactory
{
  public:
    Property<std::vector<uint32_t>> producibleUnitTypes;    // Entity types
    Property<std::vector<std::string>> producibleUnitNames; // Entity names
    Property<uint32_t> maxQueueSize;

  public:
    std::vector<uint32_t> productionQueue; // Entity types
    float currentUnitProgress = 0;

    CompUnitFactory()
    {
        productionQueue.reserve(Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE);
    }
};

} // namespace core

#endif
