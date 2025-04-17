#include <iostream>
#include "game/GameEngine.h"
#include "aion/Renderer.h"
#include "aion/ComponentRegistry.h"
#include "aion/EventLoop.h"

using namespace aion;

int main() {
    auto renderer = std::make_unique<Renderer>(800, 600, "SDL3 Game");
    auto eventLoop = std::make_unique<EventLoop>();

    ComponentRegistry::getInstance().registerComponent("EventLoop", std::move(eventLoop));
    ComponentRegistry::getInstance().registerComponent("Renderer", std::move(renderer));
    ComponentRegistry::getInstance().initAll();
    ComponentRegistry::getInstance().waitForAll();

    return 0;
}