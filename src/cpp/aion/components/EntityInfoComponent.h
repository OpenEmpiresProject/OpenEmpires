#ifndef ENTITYINFOCOMPONENT_H
#define ENTITYINFOCOMPONENT_H

#include "Component.h"

namespace aion
{
class EntityInfoComponent : public aion::Component<EntityInfoComponent>
{
  public:
    int entityType = 0;

    EntityInfoComponent(int entityType) : entityType(entityType)
    {
    }
};
} // namespace aion

#endif