#include <iostream>
#include "aion/Renderer.h"
#include "aion/SubSystemRegistry.h"
#include "aion/EventLoop.h"
#include "aion/GameState.h"
#include "game/ResourceLoader.h"
#include "aion/GraphicsRegistry.h"
#include "utils/Logger.h"

using namespace aion;
using namespace game;

int main() {
    utils::initLogger("build/logs/game.log");
    spdlog::info("Game started");
    spdlog::info("Initializing subsystems...");
    
    GameSettings settings;
    GraphicsRegistry graphicsRegistry;
    settings.setWindowDimensions(1024, 720);
    auto renderer = std::make_unique<Renderer>(settings, graphicsRegistry);
    auto eventLoop = std::make_unique<EventLoop>();
    auto resourceLoader = std::make_unique<ResourceLoader>(settings, graphicsRegistry, *(renderer.get()));

    SubSystemRegistry::getInstance().registerSubSystem("EventLoop", std::move(eventLoop));
    SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
    SubSystemRegistry::getInstance().registerSubSystem("ResourceLoader", std::move(resourceLoader));
    SubSystemRegistry::getInstance().initAll();
    SubSystemRegistry::getInstance().waitForAll();

    return 0;
}