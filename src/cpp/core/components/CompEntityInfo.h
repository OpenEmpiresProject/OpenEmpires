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
    int variation = 0;
    bool isDestroyed = false;
    bool isEnabled = true;
    int state = 0;

    CompEntityInfo() = default;

    CompEntityInfo(int entityType) : entityType(entityType)
    {
    }

    CompEntityInfo(int entityType, int variation) : entityType(entityType), variation(variation)
    {
    }
};
} // namespace core

#endif