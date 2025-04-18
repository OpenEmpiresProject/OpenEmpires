#include <iostream>
#include "game/GameEngine.h"
#include "aion/Renderer.h"
#include "aion/SubSystemRegistry.h"
#include "aion/EventLoop.h"
#include "aion/ECS.h"
#include "game/ComponentRegistrar.h"

using namespace aion;
using namespace game;

int main() {
    auto renderer = std::make_unique<Renderer>(800, 600, "SDL3 Game");
    auto eventLoop = std::make_unique<EventLoop>();
    auto ecs = std::make_unique<ECS>();
    auto componentRegistrar = std::make_unique<ComponentRegistrar>();

    SubSystemRegistry::getInstance().registerSubSystem("EventLoop", std::move(eventLoop));
    SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
    SubSystemRegistry::getInstance().registerSubSystem("ECS", std::move(ecs));
    SubSystemRegistry::getInstance().registerSubSystem("Registrar", std::move(componentRegistrar));
    SubSystemRegistry::getInstance().initAll();
    SubSystemRegistry::getInstance().waitForAll();

    return 0;
}