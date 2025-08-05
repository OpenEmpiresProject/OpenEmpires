#ifndef COMPENTITYINFO_H
#define COMPENTITYINFO_H

#include <cstdint>
#include <entt/entity/registry.hpp>

namespace core
{
class CompEntityInfo
{
  public:
    uint32_t entityId = entt::null;
    int entityType = 0;
    int entitySubType = 0;
    int variation = 0;
    bool isDestroyed = false;
    bool isEnabled = true;

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