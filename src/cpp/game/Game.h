#ifndef GAME_H
#define GAME_H

#include "CommandCenter.h"
#include "Coordinates.h"
#include "EventLoop.h"
#include "GameState.h"
#include "GraphicsLoaderFromDRS.h"
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

        ion::initLogger("logs/game.log");
        std::cout << "Starting the game2\n";

        spdlog::info("Game started");
        spdlog::info("Initializing subsystems...");

        ion::GraphicsRegistry graphicsRegistry;

        auto settings = std::make_shared<ion::GameSettings>();
        settings->setWindowDimensions(1366, 768);
        ion::ServiceRegistry::getInstance().registerService(settings);

        ion::GameState::getInstance().gameMap.init(settings->getWorldSizeInTiles().width,
                                                   settings->getWorldSizeInTiles().height);

        std::stop_source stopSource;
        std::stop_token stopToken = stopSource.get_token();

        ion::ThreadSynchronizer<ion::FrameData> simulatorRendererSynchronizer;
        // game::GraphicsLoaderFromImages graphicsLoader;
        game::GraphicsLoaderFromDRS graphicsLoader;

        auto eventLoop = std::make_shared<ion::EventLoop>(&stopToken);
        auto simulator = std::make_shared<ion::Simulator>(simulatorRendererSynchronizer, eventLoop);
        auto renderer = std::make_shared<ion::Renderer>(
            &stopSource, graphicsRegistry, simulatorRendererSynchronizer, graphicsLoader);
        auto cc = std::make_shared<ion::CommandCenter>();

        ion::ServiceRegistry::getInstance().registerService(cc);
        ion::ServiceRegistry::getInstance().registerService(simulator);

        eventLoop->registerListener(std::move(simulator));
        eventLoop->registerListener(std::move(cc));

        auto resourceLoader =
            std::make_shared<ResourceLoader>(&stopToken, settings, graphicsRegistry, renderer);

        ion::SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
        ion::SubSystemRegistry::getInstance().registerSubSystem("ResourceLoader",
                                                                std::move(resourceLoader));
        ion::SubSystemRegistry::getInstance().registerSubSystem("EventLoop", std::move(eventLoop));

        ion::SubSystemRegistry::getInstance().initAll();
        ion::SubSystemRegistry::getInstance().shutdownAll();

        spdlog::shutdown();
        spdlog::drop_all();

        return 0;
    }
};

} // namespace game

#endif