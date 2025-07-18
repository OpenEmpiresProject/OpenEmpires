#ifndef GAME_H
#define GAME_H

#include "BuildingManager.h"
#include "CommandCenter.h"
#include "Coordinates.h"
#include "DemoWorldCreator.h"
#include "EntityDefinitionLoader.h"
#include "EventLoop.h"
#include "GameState.h"
#include "GraphicsLoaderFromDRS.h"
#include "GraphicsLoaderFromImages.h"
#include "GraphicsRegistry.h"
#include "HUD.h"
#include "PlayerActionResolver.h"
#include "PlayerManager.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "ServiceRegistry.h"
#include "Simulator.h"
#include "SubSystemRegistry.h"
#include "ThreadQueue.h"
#include "ThreadSynchronizer.h"
#include "UIManager.h"
#include "UnitManager.h"
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
    struct Params
    {
        bool populateWorld = true;
        bool revealAll = false;
        std::shared_ptr<ion::EventHandler> eventHandler;
    };

    int run()
    {
        Params params{
            .populateWorld = true,
            .revealAll = false,
        };
        return runInternal(params);
    }

    int runIntegTestEnv(std::shared_ptr<ion::EventHandler> eventHandler)
    {
        Params params{.populateWorld = false, .revealAll = true, .eventHandler = eventHandler};
        return runInternal(params);
    }

  private:
    int runInternal(const Params& params)
    {
        ion::initLogger("logs/game.log");

        spdlog::info("Game starting");
        spdlog::info("Initializing subsystems...");

        ion::GraphicsRegistry graphicsRegistry;

        auto settings = std::make_shared<ion::GameSettings>();
        settings->setWindowDimensions(1366, 768);

        if (params.revealAll)
            settings->setFOWRevealStatus(ion::RevealStatus::EXPLORED);

        ion::ServiceRegistry::getInstance().registerService(settings);

        std::stop_source stopSource;
        std::stop_token stopToken = stopSource.get_token();

        ion::ThreadSynchronizer<ion::FrameData> simulatorRendererSynchronizer;
        // game::GraphicsLoaderFromImages graphicsLoader;
        game::GraphicsLoaderFromDRS graphicsLoader;

        auto gameState = std::make_shared<ion::GameState>();
        gameState->gameMap.init(settings->getWorldSizeInTiles().width,
                                settings->getWorldSizeInTiles().height);
        ion::ServiceRegistry::getInstance().registerService(gameState);

        auto coordinates = std::make_shared<ion::Coordinates>(settings);
        ion::ServiceRegistry::getInstance().registerService(coordinates);

        auto eventLoop = std::make_shared<ion::EventLoop>(&stopToken);
        auto simulator = std::make_shared<ion::Simulator>(simulatorRendererSynchronizer, eventLoop);
        ion::ServiceRegistry::getInstance().registerService(simulator);

        auto uiManager = std::make_shared<ion::UIManager>();
        ion::ServiceRegistry::getInstance().registerService(uiManager);

        auto playerManager = std::make_shared<ion::PlayerManager>();
        ion::ServiceRegistry::getInstance().registerService(playerManager);

        auto resourceManager = std::make_shared<ResourceManager>();
        ion::ServiceRegistry::getInstance().registerService(resourceManager);

        auto hud = std::make_shared<HUD>();
        ion::ServiceRegistry::getInstance().registerService(hud);

        auto renderer = std::make_shared<ion::Renderer>(
            &stopSource, graphicsRegistry, simulatorRendererSynchronizer, graphicsLoader);
        auto cc = std::make_shared<ion::CommandCenter>();
        ion::ServiceRegistry::getInstance().registerService(cc);

        auto entityDefLoader = std::make_shared<game::EntityDefinitionLoader>();
        entityDefLoader->load();
        std::shared_ptr<ion::EntityFactory> entityFactory = entityDefLoader;
        ion::ServiceRegistry::getInstance().registerService(entityFactory);

        auto buildingMngr = std::make_shared<ion::BuildingManager>();
        auto unitManager = std::make_shared<ion::UnitManager>();
        auto playerActionResolver = std::make_shared<game::PlayerActionResolver>();

        if (params.eventHandler)
            eventLoop->registerListener(params.eventHandler);

        eventLoop->registerListener(std::move(simulator));
        eventLoop->registerListener(std::move(cc));
        eventLoop->registerListener(std::move(uiManager));
        eventLoop->registerListener(std::move(playerManager));
        eventLoop->registerListener(std::move(resourceManager));
        eventLoop->registerListener(std::move(hud));
        eventLoop->registerListener(std::move(buildingMngr));
        eventLoop->registerListener(std::move(unitManager));
        eventLoop->registerListener(std::move(playerActionResolver));

        auto resourceLoader = std::make_shared<DemoWorldCreator>(
            &stopToken, settings, graphicsRegistry, renderer, params.populateWorld);

        ion::SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
        ion::SubSystemRegistry::getInstance().registerSubSystem("DemoWorldCreator",
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