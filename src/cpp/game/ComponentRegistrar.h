#ifndef COMPONENTREGISTRAR_H
#define COMPONENTREGISTRAR_H

#include "SubSystem.h"
#include "SubSystemRegistry.h"
#include "components/Physics.h"
#include "components/Graphics.h"
#include "ECS.h"

namespace game
{
    class ComponentRegistrar : public aion::SubSystem
    {
    public:
        static void registerComponentTypes()
        {
            // Register component types here
            auto ecs = (aion::ECS*)aion::SubSystemRegistry::getInstance().getSubSystem("ECS");
            ecs->registerComponent<Physics>();
            ecs->registerComponent<Graphics>();
        }

        // SubSystem methods
        void init() override
        {
            registerComponentTypes();
        }
        void shutdown() override
        {
            // Cleanup if needed
        }
    };   
}


#endif