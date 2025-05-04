#ifndef ENTITYINFOCOMPONENT_H
#define ENTITYINFOCOMPONENT_H

#include "Component.h"

namespace aion
{
class EntityInfoComponent : public aion::Component<EntityInfoComponent>
{
  public:
    int entityType = 0;
    int variation = 0;
    int debugHighlightType = static_cast<int>(utils::DebugHighlightType::NONE);
    bool isStatic = false;

    EntityInfoComponent(int entityType) : entityType(entityType)
    {
    }

    EntityInfoComponent(int entityType, int variation)
        : entityType(entityType), variation(variation)
    {
    }

    EntityInfoComponent(int entityType, int variation, bool isStatic)
        : entityType(entityType), variation(variation), isStatic(isStatic)
    {
    }

    void setDebugHighlightType(utils::DebugHighlightType type)
    {
        debugHighlightType |= static_cast<int>(type);
    }
    void clearDebugHighlightType(utils::DebugHighlightType type)
    {
        debugHighlightType &= ~static_cast<int>(type);
    }
    bool hasDebugHighlightType(utils::DebugHighlightType type) const
    {
        return (debugHighlightType & static_cast<int>(type)) != 0;
    }
    void clearAllDebugHighlightTypes()
    {
        debugHighlightType = static_cast<int>(utils::DebugHighlightType::NONE);
    }
};
} // namespace aion

#endif