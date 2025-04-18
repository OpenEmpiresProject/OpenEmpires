#include <iostream>
#include "aion/Renderer.h"
#include "aion/SubSystemRegistry.h"
#include "aion/EventLoop.h"
#include "aion/GameState.h"

using namespace aion;

int main() {
    auto renderer = std::make_unique<Renderer>(800, 600, "SDL3 Game");
    auto eventLoop = std::make_unique<EventLoop>();
    auto gameState = std::make_unique<GameState>();

    SubSystemRegistry::getInstance().registerSubSystem("EventLoop", std::move(eventLoop));
    SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
    SubSystemRegistry::getInstance().registerSubSystem("GameState", std::move(gameState));
    SubSystemRegistry::getInstance().initAll();
    SubSystemRegistry::getInstance().waitForAll();

    return 0;
}