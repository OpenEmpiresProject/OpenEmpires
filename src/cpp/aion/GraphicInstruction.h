#ifndef GRAPHICINSTRUCTION_H
#define GRAPHICINSTRUCTION_H

#include "GraphicsRegistry.h"
#include "Vec2d.h"

#include <entt/entity/registry.hpp>

namespace aion
{
class GraphicInstruction
{
  public:
    enum class Type
    {
        NONE = 0,
        ADD,
        REMOVE,
        UPDATE
    };

    const GraphicInstruction::Type type;
    const entt::entity entity = entt::null; // Entity to query GameState GraphicsComponent
    const GraphicsID graphicsID; // GraphicsID to query GraphicsRegistry to get the textures
    const Vec2d worldPosition = {0,
                                 0}; // World logical position (to be converted to screen position)

    GraphicInstruction(GraphicInstruction::Type type,
                       entt::entity entity,
                       const GraphicsID& graphicsID,
                       Vec2d worldPosition)
        : entity(entity), graphicsID(graphicsID), worldPosition(worldPosition), type(type)
    {
    }
};
} // namespace aion

#endif