#include "GraphicInstruction.h"
#include "Simulator.h"
#include "ThreadQueue.h"
#include "aion/EventLoop.h"
#include "aion/GameState.h"
#include "aion/GraphicsRegistry.h"
#include "aion/InputListener.h"
#include "aion/Renderer.h"
#include "aion/SubSystemRegistry.h"
#include "aion/Viewport.h"
#include "game/ResourceLoader.h"
#include "utils/Logger.h"

#include <iostream>
#include <readerwriterqueue.h>
#include <stop_token>
#include <vector>

using namespace aion;
using namespace game;

int runGame()
{
    std::cout << "Starting the game1\n";

    utils::initLogger("build/logs/game.log");
    std::cout << "Starting the game2\n";

    spdlog::info("Game started");
    spdlog::info("Initializing subsystems...");

    GameSettings settings;
    GraphicsRegistry graphicsRegistry;
    settings.setWindowDimensions(1900, 1024);

    std::stop_source stopSource;
    std::stop_token stopToken = stopSource.get_token();

    ThreadQueue eventQueue;
    ThreadQueue renderQueue;

    Viewport viewport(settings);

    auto eventLoop = std::make_unique<EventLoop>(&stopToken);
    auto simulator = std::make_unique<Simulator>(renderQueue);
    auto inputHandler = std::make_unique<InputListener>(eventQueue);
    auto renderer = std::make_unique<Renderer>(&stopSource, settings, graphicsRegistry, renderQueue,
                                               eventQueue, viewport);

    eventLoop->registerListener(std::move(simulator));
    eventLoop->registerListener(std::move(inputHandler));
    eventLoop->registerListenerRawPtr(&viewport);
    auto resourceLoader =
        std::make_unique<ResourceLoader>(&stopToken, settings, graphicsRegistry, *(renderer.get()));

    SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
    SubSystemRegistry::getInstance().registerSubSystem("ResourceLoader", std::move(resourceLoader));
    SubSystemRegistry::getInstance().registerSubSystem("EventLoop", std::move(eventLoop));

    SubSystemRegistry::getInstance().initAll();
    SubSystemRegistry::getInstance().waitForAll();

    spdlog::shutdown();
    spdlog::drop_all();

    return 0;
}

int main()
{
    return runGame();
}