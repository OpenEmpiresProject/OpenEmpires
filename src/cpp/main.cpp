#include "GraphicInstruction.h"
#include "Simulator.h"
#include "aion/EventLoop.h"
#include "aion/GameState.h"
#include "aion/GraphicsRegistry.h"
#include "aion/Renderer.h"
#include "aion/SubSystemRegistry.h"
#include "game/ResourceLoader.h"
#include "utils/Logger.h"

#include <iostream>
#include <readerwriterqueue.h>
#include <vector>

using namespace aion;
using namespace game;

int main()
{
    utils::initLogger("build/logs/game.log");
    spdlog::info("Game started");
    spdlog::info("Initializing subsystems...");

    GameSettings settings;
    GraphicsRegistry graphicsRegistry;
    settings.setWindowDimensions(1024, 720);

    moodycamel::ReaderWriterQueue<std::vector<GraphicInstruction>> threadQueue;

    auto renderer = std::make_unique<Renderer>(settings, graphicsRegistry, threadQueue);
    auto eventLoop = std::make_unique<EventLoop>();
    auto simulator = std::make_unique<Simulator>(threadQueue);
    eventLoop->registerListener(std::move(simulator));
    auto resourceLoader =
        std::make_unique<ResourceLoader>(settings, graphicsRegistry, *(renderer.get()));

    SubSystemRegistry::getInstance().registerSubSystem("EventLoop", std::move(eventLoop));
    SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
    SubSystemRegistry::getInstance().registerSubSystem("ResourceLoader", std::move(resourceLoader));
    SubSystemRegistry::getInstance().initAll();
    SubSystemRegistry::getInstance().waitForAll();

    return 0;
}