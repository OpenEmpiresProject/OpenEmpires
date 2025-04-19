#ifndef ACTIONCOMPONENT_H
#define ACTIONCOMPONENT_H

#include "Component.h"

namespace aion
{
    class ActionComponent : public aion::Component<ActionComponent>
    {
    public:
        // TODO: Temp
        int action = 0;

        ActionComponent(int action) : action(action) {}
    };
}

#endif