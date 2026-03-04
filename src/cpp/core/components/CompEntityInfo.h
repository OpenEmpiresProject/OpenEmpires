#ifndef COMPENTITYINFO_H
#define COMPENTITYINFO_H

#include "Property.h"

#include <cstdint>
#include <entt/entity/registry.hpp>
#include <unordered_set>

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

    uint32_t getParentEntityId() const
    {
        return parentEntityId;
    }

    const std::unordered_set<uint32_t>& getChildEntities() const
    {
        return childEntities;
    }

    CompEntityInfo() = default;

    CompEntityInfo(int entityType) : entityType(entityType)
    {
    }

    CompEntityInfo(int entityType, int variation) : entityType(entityType), variation(variation)
    {
    }

    void addChild(CompEntityInfo& child)
    {
        childEntities.insert(child.entityId);
        child.parentEntityId = entityId;
    }

  private:
    uint32_t parentEntityId = entt::null;
    std::unordered_set<uint32_t> childEntities;
};
} // namespace core

#endif