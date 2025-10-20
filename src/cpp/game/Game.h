#ifndef GAME_H
#define GAME_H

#include "BuildingManager.h"
#include "CommandCenter.h"
#include "Coordinates.h"
#include "CursorManager.h"
#include "DemoWorldCreator.h"
#include "EntityDefinitionLoader.h"
#include "EntityTypeRegistry.h"
#include "EventLoop.h"
#include "GameShortcutResolver.h"
#include "GraphicsLoaderFromDRS.h"
#include "GraphicsLoaderFromImages.h"
#include "GraphicsRegistry.h"
#include "HUDUpdater.h"
#include "PlayerController.h"
#include "PlayerFactory.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "ServiceRegistry.h"
#include "Simulator.h"
#include "StateManager.h"
#include "SubSystemRegistry.h"
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
        std::shared_ptr<core::EventHandler> eventHandler;
    };

    int run()
    {
        Params params{
            .populateWorld = true,
            .revealAll = false,
        };
        return runInternal(params);
    }

    int runIntegTestEnv(std::shared_ptr<core::EventHandler> eventHandler)
    {
        Params params{.populateWorld = false, .revealAll = true, .eventHandler = eventHandler};
        return runInternal(params);
    }

  private:
    int runInternal(const Params& params)
    {
        core::initLogger("logs/game.log");

        spdlog::info("Game starting");
        spdlog::info("Initializing subsystems...");

        core::GraphicsRegistry graphicsRegistry;

        auto settings = std::make_shared<core::Settings>();
        settings->setWindowDimensions(1366, 768);

        if (params.revealAll)
            settings->setFOWRevealStatus(core::RevealStatus::EXPLORED);

        core::ServiceRegistry::getInstance().registerService(settings);

        std::stop_source stopSource;
        std::stop_token stopToken = stopSource.get_token();

        core::ThreadSynchronizer<core::FrameData> simulatorRendererSynchronizer;
        // game::GraphicsLoaderFromImages graphicsLoader;
        game::GraphicsLoaderFromDRS graphicsLoader;

        auto stateMan = std::make_shared<core::StateManager>();
        stateMan->gameMap().init(settings->getWorldSizeInTiles().width,
                                 settings->getWorldSizeInTiles().height);
        core::ServiceRegistry::getInstance().registerService(stateMan);

        auto coordinates = std::make_shared<core::Coordinates>(settings);
        core::ServiceRegistry::getInstance().registerService(coordinates);

        auto eventLoop = std::make_shared<core::EventLoop>(&stopToken);
        auto simulator = std::make_shared<core::Simulator>(simulatorRendererSynchronizer);
        core::ServiceRegistry::getInstance().registerService(simulator);

        auto uiManager = std::make_shared<core::UIManager>();
        core::ServiceRegistry::getInstance().registerService(uiManager);

        auto playerManager = std::make_shared<core::PlayerFactory>();
        core::ServiceRegistry::getInstance().registerService(playerManager);

        auto playerController = std::make_shared<core::PlayerController>();
        core::ServiceRegistry::getInstance().registerService(playerController);

        auto resourceManager = std::make_shared<ResourceManager>();
        core::ServiceRegistry::getInstance().registerService(resourceManager);

        auto entityTypeRegistry = std::make_shared<core::EntityTypeRegistry>();
        core::ServiceRegistry::getInstance().registerService(entityTypeRegistry);

        auto hud = std::make_shared<HUDUpdater>();
        core::ServiceRegistry::getInstance().registerService(hud);

        auto renderer = std::make_shared<core::Renderer>(
            &stopSource, graphicsRegistry, simulatorRendererSynchronizer, graphicsLoader);
        auto cc = std::make_shared<core::CommandCenter>();
        core::ServiceRegistry::getInstance().registerService(cc);

        auto entityDefLoader = std::make_shared<game::EntityDefinitionLoader>();
        entityDefLoader->load();
        std::shared_ptr<core::EntityFactory> entityFactory = entityDefLoader;
        core::ServiceRegistry::getInstance().registerService(entityFactory);

        auto buildingMngr = std::make_shared<core::BuildingManager>();
        auto unitManager = std::make_shared<core::UnitManager>();

        auto gameShortcutResolver = std::make_shared<game::GameShortcutResolver>();
        std::shared_ptr<core::ShortcutResolver> shortcutResolver = gameShortcutResolver;
        core::ServiceRegistry::getInstance().registerService(shortcutResolver);

        std::unordered_map<core::CursorType, core::GraphicsID> cursorMap;

        core::GraphicsID defaultIcon;
        defaultIcon.entityType = game::EntityTypes::ET_UI_ELEMENT;
        defaultIcon.entitySubType = game::EntitySubTypes::UI_CURSOR;
        defaultIcon.variation = 0;

        core::GraphicsID buildIcon;
        buildIcon.entityType = game::EntityTypes::ET_UI_ELEMENT;
        buildIcon.entitySubType = game::EntitySubTypes::UI_CURSOR;
        buildIcon.variation = 7;

        core::GraphicsID assignTaskCursor;
        assignTaskCursor.entityType = game::EntityTypes::ET_UI_ELEMENT;
        assignTaskCursor.entitySubType = game::EntitySubTypes::UI_CURSOR;
        assignTaskCursor.variation = 3;

        cursorMap[core::CursorType::DEFAULT_INGAME] = defaultIcon;
        cursorMap[core::CursorType::BUILD] = buildIcon;
        cursorMap[core::CursorType::ASSIGN_TASK] = assignTaskCursor;
        auto cursorManager = std::make_shared<core::CursorManager>(cursorMap);
        core::ServiceRegistry::getInstance().registerService(cursorManager);

        if (params.eventHandler)
            eventLoop->registerListener(params.eventHandler);

        eventLoop->registerListener(std::move(simulator));
        eventLoop->registerListener(std::move(cc));
        eventLoop->registerListener(std::move(uiManager));
        eventLoop->registerListener(std::move(playerController));
        eventLoop->registerListener(std::move(resourceManager));
        eventLoop->registerListener(std::move(hud));
        eventLoop->registerListener(std::move(buildingMngr));
        eventLoop->registerListener(std::move(unitManager));
        eventLoop->registerListener(std::move(cursorManager));

        auto resourceLoader =
            std::make_shared<DemoWorldCreator>(&stopToken, settings, params.populateWorld);

        core::SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
        core::SubSystemRegistry::getInstance().registerSubSystem("DemoWorldCreator",
                                                                 std::move(resourceLoader));
        core::SubSystemRegistry::getInstance().registerSubSystem("EventLoop", std::move(eventLoop));

        core::SubSystemRegistry::getInstance().initAll();
        core::SubSystemRegistry::getInstance().shutdownAll();

        spdlog::shutdown();
        spdlog::drop_all();

        return 0;
    }
};

} // namespace game

#endif