#ifndef ENTITYINFOCOMPONENT_H
#define ENTITYINFOCOMPONENT_H

#include <entt/entity/registry.hpp>

namespace aion
{
class EntityInfoComponent
{
  public:
    int entityType = 0;
    int variation = 0;

    EntityInfoComponent(int entityType) : entityType(entityType)
    {
    }

    EntityInfoComponent(int entityType, int variation)
        : entityType(entityType), variation(variation)
    {
    }
};
} // namespace aion

#endif