#ifndef GAME_H
#define GAME_H

#include "CommandCenter.h"
#include "Coordinates.h"
#include "EventLoop.h"
#include "GameState.h"
#include "GraphicsLoaderFromImages.h"
#include "GraphicsRegistry.h"
#include "Renderer.h"
#include "ResourceLoader.h"
#include "ServiceRegistry.h"
#include "Simulator.h"
#include "SubSystemRegistry.h"
#include "ThreadQueue.h"
#include "ThreadSynchronizer.h"
#include "utils/Logger.h"
#include "utils/Types.h"

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

        aion::initLogger("build/logs/game.log");
        std::cout << "Starting the game2\n";

        spdlog::info("Game started");
        spdlog::info("Initializing subsystems...");

        aion::GraphicsRegistry graphicsRegistry;

        auto settings = std::make_shared<aion::GameSettings>();
        settings->setWindowDimensions(1366, 768);
        aion::ServiceRegistry::getInstance().registerService(settings);

        std::stop_source stopSource;
        std::stop_token stopToken = stopSource.get_token();

        aion::ThreadSynchronizer<aion::FrameData> simulatorRendererSynchronizer;
        aion::GraphicsLoaderFromImages graphicsLoader;

        auto eventLoop = std::make_shared<aion::EventLoop>(&stopToken);
        auto simulator =
            std::make_shared<aion::Simulator>(simulatorRendererSynchronizer, eventLoop);
        auto renderer = std::make_shared<aion::Renderer>(
            &stopSource, graphicsRegistry, simulatorRendererSynchronizer, graphicsLoader);
        auto cc = std::make_shared<aion::CommandCenter>();

        aion::ServiceRegistry::getInstance().registerService(cc);
        aion::ServiceRegistry::getInstance().registerService(simulator);

        eventLoop->registerListener(std::move(simulator));
        eventLoop->registerListener(std::move(cc));

        auto resourceLoader =
            std::make_shared<ResourceLoader>(&stopToken, settings, graphicsRegistry, renderer);

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