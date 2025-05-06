#ifndef GAME_H
#define GAME_H

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

namespace game
{
class Game
{
  public:
    int run()
    {
        std::cout << "Starting the game1\n";

        utils::initLogger("build/logs/game.log");
        std::cout << "Starting the game2\n";

        spdlog::info("Game started");
        spdlog::info("Initializing subsystems...");

        aion::GameSettings settings;
        aion::GraphicsRegistry graphicsRegistry;
        settings.setWindowDimensions(1366, 768);

        std::stop_source stopSource;
        std::stop_token stopToken = stopSource.get_token();

        aion::ThreadQueue renderQueue;

        aion::Viewport viewport(settings);
        viewport.setViewportPositionInPixels(viewport.feetToPixels(aion::Vec2d(0, 0)));

        auto eventLoop = std::make_unique<aion::EventLoop>(&stopToken);
        auto simulator = std::make_unique<aion::Simulator>(renderQueue, viewport);
        auto inputHandler = std::make_unique<aion::InputListener>();
        auto renderer = std::make_unique<aion::Renderer>(&stopSource, settings, graphicsRegistry,
                                                         renderQueue, viewport);

        eventLoop->registerListener(std::move(simulator));
        eventLoop->registerListener(std::move(inputHandler));
        auto resourceLoader = std::make_unique<ResourceLoader>(&stopToken, settings,
                                                               graphicsRegistry, *(renderer.get()));

        aion::SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
        aion::SubSystemRegistry::getInstance().registerSubSystem("ResourceLoader",
                                                                 std::move(resourceLoader));
        aion::SubSystemRegistry::getInstance().registerSubSystem("EventLoop", std::move(eventLoop));

        aion::SubSystemRegistry::getInstance().initAll();
        aion::SubSystemRegistry::getInstance().shutdownAll();

        spdlog::shutdown();
        spdlog::drop_all();

        return 0;
    }
};

} // namespace game

#endif