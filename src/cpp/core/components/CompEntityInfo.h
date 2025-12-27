#ifndef COMPENTITYINFO_H
#define COMPENTITYINFO_H

#include "Property.h"

#include <cstdint>
#include <entt/entity/registry.hpp>

namespace core
{
class CompEntityInfo
{
  public:
    Property<uint32_t> entityId{(uint32_t) entt::null};
    Property<uint32_t> entityType = 0;
    Property<std::string> entityName;

  public:
    // TODO: entitySubType cannot be convert to a Property yet it is forcefully
    // changed to show chopped trees in ResourceManager. Once it is handled
    // properly this to be converted into a Property.
    int entitySubType = 0;
    int variation = 0;
    bool isDestroyed = false;
    bool isEnabled = true;
    int state = 0;

    CompEntityInfo() = default;

    CompEntityInfo(int entityType) : entityType(entityType)
    {
    }

    CompEntityInfo(int entityType, int entitySubType, int variation)
        : entityType(entityType), variation(variation), entitySubType(entitySubType)
    {
    }
};
} // namespace core

#endif