#ifndef GRAPHICSCOMPONENT_H
#define GRAPHICSCOMPONENT_H

#include "Component.h"
#include <SDL3/SDL.h>
#include "GraphicsRegistry.h"

namespace aion
{
    class GraphicsComponent : public aion::Component<GraphicsComponent>
    {
    public:
        aion::GraphicsID graphicID; // ID to look up the graphic in the registry
        int currentFrame = 0;

        GraphicsComponent(const aion::GraphicsID& id, int frame) : graphicID(id), currentFrame(frame) {}
    };
}

#endif