#ifndef GAME_H
#define GAME_H

#include "BuildingManager.h"
#include "CommandCenter.h"
#include "Coordinates.h"
#include "CursorManager.h"
#include "DRSGraphicsLoader.h"
#include "DemoWorldCreator.h"
#include "EntityModelLoaderV2.h"
#include "EntityTypeRegistry.h"
#include "EventLoop.h"
#include "GameShortcutResolver.h"
#include "GraphicsInstructor.h"
#include "GraphicsRegistry.h"
#include "HUDUpdater.h"
#include "LogLevelController.h"
#include "PlayerController.h"
#include "PlayerFactory.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "ServiceRegistry.h"
#include "SpecialBuildingManager.h"
#include "StateManager.h"
#include "SubSystemRegistry.h"
#include "ThreadSynchronizer.h"
#include "UIManager.h"
#include "UnitManager.h"
#include "VisionSystem.h"
#include "logging/Logger.h"
#include "utils/Types.h"

#include <iostream>
#include <pybind11/embed.h>
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
        core::Ref<core::EventHandler> eventHandler;
        core::Ref<game::WorldCreator> worldCreator;

        Params()
        {
            DemoWorldCreator::Params params;
            worldCreator = core::CreateRef<DemoWorldCreator>(params);
        }
    };

    int run(const Params& params)
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
        game::DRSGraphicsLoader graphicsLoader;

        auto stateMan = std::make_shared<core::StateManager>();
        stateMan->init();
        core::ServiceRegistry::getInstance().registerService(stateMan);

        auto coordinates = std::make_shared<core::Coordinates>(settings);
        core::ServiceRegistry::getInstance().registerService(coordinates);

        auto eventLoop = std::make_shared<core::EventLoop>(&stopToken);
        auto simulator = std::make_shared<core::GraphicsInstructor>(simulatorRendererSynchronizer);
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

        auto drsInterface = std::make_shared<game::DRSInterface>();
        auto entityDefLoader = std::make_shared<game::EntityModelLoaderV2>(
            "assets/scripts", "model_importer", drsInterface);
        entityDefLoader->init();
        std::shared_ptr<core::EntityFactory> entityFactory = entityDefLoader;
        std::shared_ptr<core::GraphicsLoadupDataProvider> graphicProvider = entityDefLoader;
        core::ServiceRegistry::getInstance().registerService(entityFactory);
        core::ServiceRegistry::getInstance().registerService(graphicProvider);

        auto buildingMngr = std::make_shared<core::BuildingManager>();
        auto unitManager = std::make_shared<core::UnitManager>();

        auto gameShortcutResolver = std::make_shared<game::GameShortcutResolver>();
        std::shared_ptr<core::ShortcutResolver> shortcutResolver = gameShortcutResolver;
        core::ServiceRegistry::getInstance().registerService(shortcutResolver);

        auto cursorEntityType = entityTypeRegistry->getEntityType("cursor");
        core::GraphicsID defaultIcon;
        defaultIcon.entityType = cursorEntityType;
        /*defaultIcon.entityType = game::EntityTypes::ET_UI_ELEMENT;
        defaultIcon.uiElementType = (int) game::UIElementTypes::CURSOR;*/
        defaultIcon.variation = 0;

        core::GraphicsID buildIcon;
        defaultIcon.entityType = cursorEntityType;

        //         buildIcon.entityType = game::EntityTypes::ET_UI_ELEMENT;
        //         buildIcon.uiElementType = (int) game::UIElementTypes::CURSOR;
        buildIcon.variation = 7;

        core::GraphicsID assignTaskCursor;
        assignTaskCursor.entityType = cursorEntityType;

        //         assignTaskCursor.entityType = game::EntityTypes::ET_UI_ELEMENT;
        //         assignTaskCursor.uiElementType = (int) game::UIElementTypes::CURSOR;
        assignTaskCursor.variation = 3;

        core::GraphicsID garrisonCursor;
        garrisonCursor.entityType = cursorEntityType;
        //
        //         garrisonCursor.entityType = game::EntityTypes::ET_UI_ELEMENT;
        //         garrisonCursor.uiElementType = (int) game::UIElementTypes::CURSOR;
        garrisonCursor.variation = 13;

        entityTypeRegistry->registerCursorGraphic(core::CursorType::DEFAULT_INGAME, defaultIcon);
        entityTypeRegistry->registerCursorGraphic(core::CursorType::BUILD, buildIcon);
        entityTypeRegistry->registerCursorGraphic(core::CursorType::ASSIGN_TASK, assignTaskCursor);
        entityTypeRegistry->registerCursorGraphic(core::CursorType::GARRISON, garrisonCursor);

        auto cursorManager = std::make_shared<core::CursorManager>();
        core::ServiceRegistry::getInstance().registerService(cursorManager);

        auto logController = std::make_shared<core::LogLevelController>();
        auto visionSystem = std::make_shared<core::VisionSystem>();
        auto specialBuildingManager = std::make_shared<game::SpecialBuildingManager>();

        if (params.eventHandler)
            eventLoop->registerListener(params.eventHandler);

        core::ServiceRegistry::getInstance().registerService(params.worldCreator);

        eventLoop->registerListener(std::move(simulator));
        eventLoop->registerListener(std::move(cc));
        eventLoop->registerListener(std::move(uiManager));
        eventLoop->registerListener(std::move(playerController));
        eventLoop->registerListener(std::move(resourceManager));
        eventLoop->registerListener(std::move(hud));
        eventLoop->registerListener(std::move(buildingMngr));
        eventLoop->registerListener(std::move(unitManager));
        eventLoop->registerListener(std::move(cursorManager));
        eventLoop->registerListener(std::move(logController));
        eventLoop->registerListener(std::move(visionSystem));
        eventLoop->registerListener(std::move(specialBuildingManager));
        eventLoop->registerListener(params.worldCreator);

        core::SubSystemRegistry::getInstance().registerSubSystem("Renderer", std::move(renderer));
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