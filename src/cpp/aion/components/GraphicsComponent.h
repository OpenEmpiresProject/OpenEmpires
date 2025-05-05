#ifndef GRAPHICSCOMPONENT_H
#define GRAPHICSCOMPONENT_H

#include "GraphicsRegistry.h"
#include "WidthHeight.h"

#include <SDL3/SDL.h>

namespace aion
{
// Component will be owned by the Simulator
class GraphicsComponent : public GraphicsID
{
  public:
    entt::entity entityID = entt::null;
    Vec2d positionInFeet = {0, 0};
    int debugHighlightType = static_cast<int>(utils::DebugHighlightType::NONE);
    bool isStatic = false;

    // add set and clear functions to manipulate the debugHighlightType flag
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