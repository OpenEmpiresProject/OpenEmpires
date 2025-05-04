#ifndef GRAPHICINSTRUCTION_H
#define GRAPHICINSTRUCTION_H

#include "GraphicsRegistry.h"
#include "Vec2d.h"

#include <entt/entity/registry.hpp>

namespace aion
{
struct GraphicInstruction
{
    enum class Type
    {
        NONE = 0,
        ADD,
        REMOVE,
        UPDATE
    };

    GraphicInstruction::Type type;
    entt::entity entity = entt::null;
    Vec2d positionInFeet = {0, 0};
    int entityType = 0;
    int entitySubType = 0;
    int variation = 0;
    int action = 0;
    utils::Direction direction = utils::Direction::NORTH;
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