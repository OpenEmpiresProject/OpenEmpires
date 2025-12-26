#include "DemoWorldCreator.h"

#include "Coordinates.h"
#include "EntityFactory.h"
#include "EntityTypeRegistry.h"
#include "GameTypes.h"
#include "PlayerController.h"
#include "PlayerFactory.h"
#include "Renderer.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "SubSystemRegistry.h"
#include "Tile.h"
#include "UI.h"
#include "UIManager.h"
#include "commands/CmdIdle.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "logging/Logger.h"
#include "utils/ObjectPool.h"
#include "utils/Types.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <memory>
#include <random>
#include <string>

#define WITH(statement) if (statement)

namespace fs = std::filesystem;
using namespace game;
using namespace core;
using namespace drs;
using namespace std;

DemoWorldCreator::DemoWorldCreator(const Params& params) : WorldCreator(params)
{
}

bool DemoWorldCreator::isReady() const
{
    return m_isReady;
}

void DemoWorldCreator::onInit(EventLoop& eventLoop)
{
    create();
}

void DemoWorldCreator::create()
{
    spdlog::info("Creating the demo world");

    InGameResource::registerResourceType(ResourceType::WOOD);
    InGameResource::registerResourceType(ResourceType::STONE);
    InGameResource::registerResourceType(ResourceType::GOLD);

    createTerrain();
    generateRandomForest();
    createMiningCluster(getResourceType("stone"), 30, 30, 4);
    createMiningCluster(getResourceType("gold"), 20, 30, 4);
    auto players = createPlayers();
    createVillager(players[0], Tile(20, 20));
    createVillager(players[1], Tile(25, 25));
    registerVillagerActions();
    createHUD();

    m_isReady = true;

    spdlog::info("Demo world created");
}

void DemoWorldCreator::createTree(uint32_t x, uint32_t y)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto tree = factory->createEntity(EntityTypes::ET_TREE);
    auto [transform, selectible, info] =
        stateMan->getComponents<CompTransform, CompSelectible, CompEntityInfo>(tree);

    int variation = rand() % 10;

    transform.position = Feet(x * 256 + 128, y * 256 + 128);
    info.variation = variation;

    auto& map = m_stateMan->gameMap();

    map.addEntity(MapLayerType::STATIC, Tile(x, y), tree);

    // Add shadow
    //     auto shadow = factory->createEntity(EntityTypes::ET_TREE,
    //     EntitySubTypes::EST_TREE_SHADOW); auto [transformShadow, infoShadow] =
    //         stateMan->getComponents<CompTransform, CompEntityInfo>(shadow);
    //
    //     infoShadow.variation = variation;
    //     transformShadow.position = Feet(x * 256 + 128, y * 256 + 128);
    //
    //     map.addEntity(MapLayerType::ON_GROUND, Tile(x, y), shadow);
}

void DemoWorldCreator::createStoneOrGold(uint32_t entityType, uint32_t x, uint32_t y)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto entity = factory->createEntity(entityType);
    auto [transform, selectible, info] =
        stateMan->getComponents<CompTransform, CompSelectible, CompEntityInfo>(entity);

    transform.position = Feet(x * 256 + 128, y * 256 + 128);
    info.variation = rand() % 7;

    m_stateMan->gameMap().addEntity(MapLayerType::STATIC, Tile(x, y), entity);
}

void DemoWorldCreator::createVillager(Ref<core::Player> player, const Tile& tilePos)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto villager = factory->createEntity(EntityTypes::ET_VILLAGER);
    auto [transform, unit, selectible, playerComp, vision] =
        stateMan->getComponents<CompTransform, CompUnit, CompSelectible, CompPlayer, CompVision>(
            villager);

    transform.position = Feet(tilePos.x * 256 + 128, tilePos.x * 256 + 50);
    transform.face(Direction::SOUTH);
    selectible.selectionIndicator = {GraphicAddon::Type::ISO_CIRCLE,
                                     GraphicAddon::IsoCircle{10, Vec2(0, 0)}};
    playerComp.player = player;

    auto newTile = transform.position.toTile();
    stateMan->gameMap().addEntity(MapLayerType::UNITS, newTile, villager);

    player->getFogOfWar()->markAsExplored(transform.position, vision.lineOfSight);
}

void DemoWorldCreator::createMiningCluster(uint32_t entityType,
                                           uint32_t xHint,
                                           uint32_t yHint,
                                           uint8_t amount)
{
    if (amount == 0)
        return;

    auto& gameMap = m_stateMan->gameMap();

    const auto width = gameMap.width;
    const auto height = gameMap.height;

    std::vector<std::pair<uint32_t, uint32_t>> cluster;
    std::vector<std::pair<uint32_t, uint32_t>> frontier;

    auto isValid = [&](uint32_t x, uint32_t y)
    { return x < width && y < height && !gameMap.isOccupied(MapLayerType::STATIC, Tile(x, y)); };

    // Try to start from the hint or nearby
    if (!isValid(xHint, yHint))
    {
        bool found = false;
        for (int dx = -5; dx <= 5 && !found; ++dx)
        {
            for (int dy = -5; dy <= 5 && !found; ++dy)
            {
                uint32_t nx = xHint + dx;
                uint32_t ny = yHint + dy;
                if (isValid(nx, ny))
                {
                    xHint = nx;
                    yHint = ny;
                    found = true;
                }
            }
        }
        if (!isValid(xHint, yHint))
            return; // Failed to find starting point
    }

    std::mt19937 rng(std::random_device{}());

    cluster.emplace_back(xHint, yHint);
    frontier.emplace_back(xHint, yHint);

    size_t placed = 0;
    while (!frontier.empty() && placed < amount)
    {
        // Pick random tile from the frontier
        std::uniform_int_distribution<size_t> dist(0, frontier.size() - 1);
        size_t idx = dist(rng);
        auto [cx, cy] = frontier[idx];
        std::swap(frontier[idx], frontier.back());
        frontier.pop_back();

        if (!isValid(cx, cy))
            continue;

        // Place a stone here
        createStoneOrGold(entityType, cx, cy);
        ++placed;

        cluster.emplace_back(cx, cy);

        // Add neighbors to frontier
        std::vector<std::pair<uint32_t, uint32_t>> neighbors = {
            {cx + 1, cy}, {cx - 1, cy}, {cx, cy + 1}, {cx, cy - 1}};

        for (auto [nx, ny] : neighbors)
        {
            if (isValid(nx, ny))
            {
                frontier.emplace_back(nx, ny);
            }
        }
    }
}

void DemoWorldCreator::generateRandomForest()
{
    auto& gameMap = m_stateMan->gameMap();

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> posX(0, gameMap.width - 1);
    std::uniform_int_distribution<int> posY(0, gameMap.height - 1);

    int totalTiles = gameMap.width * gameMap.height;
    int targetTreeTiles = totalTiles / 6; // 25% of map should have trees
    int treesPlaced = 0;

    // Define the middle 5x5 area boundaries
    int midXStart = gameMap.width / 2 - 10;
    int midXEnd = gameMap.width / 2 + 10;
    int midYStart = gameMap.height / 2 - 10;
    int midYEnd = gameMap.height / 2 + 10;

    // 1. Place tree clusters (forests)
    while (treesPlaced < targetTreeTiles * 0.95) // Majority in forests
    {
        int clusterSize = 5 + rng() % 20; // Random cluster size: 5~25
        int centerX = posX(rng);
        int centerY = posY(rng);

        for (int i = 0; i < clusterSize; ++i)
        {
            int dx = (rng() % 5) - 2; // Random offset -2..+2
            int dy = (rng() % 5) - 2;

            int x = centerX + dx;
            int y = centerY + dy;

            if (x >= 0 && x < gameMap.width && y >= 0 && y < gameMap.height)
            {
                // Skip the middle 5x5 area
                if (x >= midXStart && x <= midXEnd && y >= midYStart && y <= midYEnd)
                    continue;

                if (gameMap.isOccupied(MapLayerType::STATIC, {x, y}) == false)
                {
                    createTree(x, y);
                    ++treesPlaced;
                }
            }
        }
    }

    // 2. Place isolated trees
    while (treesPlaced < targetTreeTiles)
    {
        int x = posX(rng);
        int y = posY(rng);

        // Skip the middle 5x5 area
        if (x >= midXStart && x <= midXEnd && y >= midYStart && y <= midYEnd)
            continue;

        if (gameMap.isOccupied(MapLayerType::STATIC, {x, y}) == false)
        {
            createTree(x, y);
            ++treesPlaced;
        }
    }
}

void DemoWorldCreator::createTile(uint32_t x, uint32_t y, EntityTypes entityType)
{
    auto size = m_settings->getWorldSizeInTiles();

    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto entity = factory->createEntity(entityType);
    auto [transform, info] = m_stateMan->getComponents<CompTransform, CompEntityInfo>(entity);

    int tc = 10;
    // Convet our top corner based coordinate to left corner based coordinate
    int newX = size.height - y;
    int newY = x;
    // AOE2 standard tiling rule. From OpenAge documentation
    int tileVariation = (newX % tc) + ((newY % tc) * tc) + 1;

    transform.position = Feet(x * 256, y * 256);
    info.variation = tileVariation;

    m_stateMan->gameMap().addEntity(MapLayerType::GROUND, Tile(x, y), entity);
}

void DemoWorldCreator::createHUD()
{
    auto typeReg = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();
    const auto genericWidgetEntityType = typeReg->getEntityType("generic_widget");

    // ui::Widget::s_entityType = EntityTypes::ET_UI_ELEMENT;
    GraphicsID resourcePanelBackground;
    resourcePanelBackground.entityType = typeReg->getEntityType("resource_panel");
    // resourcePanelBackground.entityType = EntityTypes::ET_UI_ELEMENT;
    // resourcePanelBackground.uiElementType = (int) UIElementTypes::RESOURCE_PANEL;

    auto window = CreateRef<ui::Window>(factory->createEntity(genericWidgetEntityType));
    WITH(window->withName("resourcePanel")->withBackgroundImage(resourcePanelBackground))
    {
        ServiceRegistry::getInstance().getService<UIManager>()->registerWindow(window);

        window->createChild<ui::Label>(factory->createEntity(genericWidgetEntityType))
            ->withText("0")
            ->withRect(Rect<int>(35, 5, 50, 20))
            ->withName("wood");

        window->createChild<ui::Label>(factory->createEntity(genericWidgetEntityType))
            ->withText("0")
            ->withRect(Rect<int>(195, 5, 50, 20))
            ->withName("gold");

        window->createChild<ui::Label>(factory->createEntity(genericWidgetEntityType))
            ->withText("0")
            ->withRect(Rect<int>(265, 5, 50, 20))
            ->withName("stone");

        window->createChild<ui::Label>(factory->createEntity(genericWidgetEntityType))
            ->withText("0")
            ->withRect(Rect<int>(350, 5, 50, 20))
            ->withName("population");

        window->createChild<ui::Label>(factory->createEntity(genericWidgetEntityType))
            ->withTextColor(core::Color::WHITE)
            ->withRect(Rect<int>(420, 5, 50, 20))
            ->withName("player");
    }

    WITH(auto controlPanel = CreateRef<ui::Window>(factory->createEntity(genericWidgetEntityType)))
    {
        GraphicsID controlPanelBackground;
        controlPanelBackground.entityType = typeReg->getEntityType("control_panel");
        /*controlPanelBackground.entityType = EntityTypes::ET_UI_ELEMENT;
        controlPanelBackground.uiElementType = (int) UIElementTypes::CONTROL_PANEL;*/

        controlPanel->withName("controlPanel")
            ->withBackgroundImage(controlPanelBackground)
            ->withRect(Rect<int>(0, -1, 675, 175)); // -1 is to attach to bottom of screen
        ServiceRegistry::getInstance().getService<UIManager>()->registerWindow(controlPanel);

        WITH(auto hLayout = controlPanel->createChild<ui::Layout>(
                 factory->createEntity(genericWidgetEntityType)))
        {
            WITH(
                auto commandsLayout =
                    hLayout->createChild<ui::Layout>(factory->createEntity(genericWidgetEntityType))
                        ->withSize(275, 0))
            {
            }

            WITH(
                auto infoLayout =
                    hLayout->createChild<ui::Layout>(factory->createEntity(genericWidgetEntityType))
                        ->withDirection(ui::LayoutDirection::Horizontal)
                        ->withSpacing(10)
                        ->withMargin(20)
                        ->withSize(400, 0))
            {

                WITH(auto basicInfoLayout = infoLayout
                                                ->createChild<ui::Layout>(
                                                    factory->createEntity(genericWidgetEntityType))
                                                ->withDirection(ui::LayoutDirection::Vertical)
                                                ->withSize(80, 0))
                {
                    basicInfoLayout
                        ->createChild<ui::Label>(factory->createEntity(genericWidgetEntityType))
                        ->withTextColor(core::Color::BLACK)
                        ->withSize(80, 20)
                        ->withName("selected_name")
                        ->withVisible(false);
                    basicInfoLayout
                        ->createChild<ui::Label>(factory->createEntity(genericWidgetEntityType))
                        ->withSize(m_iconSize, m_iconSize)
                        ->withName("selected_icon")
                        ->withVisible(false);
                }

                WITH(auto unitCreationLayout = infoLayout
                                                   ->createChild<ui::Layout>(factory->createEntity(
                                                       genericWidgetEntityType))
                                                   ->withDirection(ui::LayoutDirection::Vertical)
                                                   ->withLeftMargin(20)
                                                   ->withTopMargin(5)
                                                   ->withSpacing(10)
                                                   ->withSize(250, 0))
                {
                    WITH(auto garrisonLayout = unitCreationLayout
                                                   ->createChild<ui::Layout>(factory->createEntity(
                                                       genericWidgetEntityType))
                                                   ->withDirection(ui::LayoutDirection::Horizontal)
                                                   ->withSpacing(5)
                                                   ->withName("garrisoned_units_row")
                                                   ->withSize(0, m_iconSize))
                    {
                        for (int i = 0; i < Constants::ABSOLUTE_MAX_UNIT_GARRISON_SIZE; ++i)
                        {
                            garrisonLayout
                                ->createChild<ui::Label>(
                                    factory->createEntity(genericWidgetEntityType))
                                ->withSize(m_iconSize, m_iconSize)
                                ->withName(fmt::format("garrisoned_unit{}", i))
                                ->withVisible(false);
                        }
                    }

                    WITH(auto currentInProgressDetailsLayout =
                             unitCreationLayout
                                 ->createChild<ui::Layout>(
                                     factory->createEntity(genericWidgetEntityType))
                                 ->withDirection(ui::LayoutDirection::Horizontal)
                                 ->withSpacing(10)
                                 ->withName("creation_in_progress_group")
                                 ->withSize(0, 60))
                    {
                        currentInProgressDetailsLayout
                            ->createChild<ui::Label>(factory->createEntity(genericWidgetEntityType))
                            ->withSize(m_iconSize, 0)
                            ->withName("unit_creating_icon");

                        const int barWithoutIconWidth = 250 - m_iconSize - 10 /*spacing*/;

                        WITH(auto progressBarLayout =
                                 currentInProgressDetailsLayout
                                     ->createChild<ui::Layout>(
                                         factory->createEntity(genericWidgetEntityType))
                                     ->withDirection(ui::LayoutDirection::Vertical)
                                     ->withName("progress_bar_noerror_group")
                                     ->withSize(barWithoutIconWidth, 0))
                        {
                            // Two labels to display the progress and item in two lines. eg:
                            // Creating - 50%
                            // Villager
                            //
                            progressBarLayout
                                ->createChild<ui::Label>(
                                    factory->createEntity(genericWidgetEntityType))
                                ->withTextColor(core::Color::BLACK)
                                ->withSize(0, 20)
                                ->withName("progress_label");

                            progressBarLayout
                                ->createChild<ui::Label>(
                                    factory->createEntity(genericWidgetEntityType))
                                ->withTextColor(core::Color::BLACK)
                                ->withSize(0, 20)
                                ->withName("progress_item_name");

                            GraphicsID progressBarBackground;
                            progressBarBackground.entityType =
                                typeReg->getEntityType("progress_bar");
                            /*progressBarBackground.entityType = EntityTypes::ET_UI_ELEMENT;
                            progressBarBackground.uiElementType =
                                (int) UIElementTypes::PROGRESS_BAR;*/
                            progressBarLayout
                                ->createChild<ui::Label>(
                                    factory->createEntity(genericWidgetEntityType))
                                ->withBackgroundImage(progressBarBackground)
                                ->withName("progress_bar_label")
                                ->withSize(0, 10);
                        }

                        currentInProgressDetailsLayout
                            ->createChild<ui::Label>(factory->createEntity(genericWidgetEntityType))
                            ->withTextColor(core::Color::RED)
                            ->withSize(barWithoutIconWidth, 20)
                            ->withName("progress_bar_error_label")
                            ->withVisible(false);

                        currentInProgressDetailsLayout->setVisible(false);
                    }

                    WITH(auto restOfUnitQueuedLayout =
                             unitCreationLayout
                                 ->createChild<ui::Layout>(
                                     factory->createEntity(genericWidgetEntityType))
                                 ->withDirection(ui::LayoutDirection::Horizontal)
                                 ->withSpacing(5)
                                 ->withName("creation_queue_group")
                                 ->withSize(0, m_iconSize))
                    {
                        for (int i = 0; i < Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE; ++i)
                        {
                            restOfUnitQueuedLayout
                                ->createChild<ui::Label>(
                                    factory->createEntity(genericWidgetEntityType))
                                ->withSize(m_iconSize, m_iconSize)
                                ->withName(fmt::format("queued_unit_icon_{}", i))
                                ->withVisible(false);
                        }
                    }
                }
            }
        }
    }
}

std::vector<Ref<Player>> DemoWorldCreator::createPlayers()
{
    auto playerManager = ServiceRegistry::getInstance().getService<PlayerFactory>();
    auto player = playerManager->createPlayer();
    auto player2 = playerManager->createPlayer();
    auto playercontroller = ServiceRegistry::getInstance().getService<PlayerController>();
    playercontroller->setPlayer(player);

    return {player, player2};
}

void DemoWorldCreator::createTerrain()
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();

    auto size = m_settings->getWorldSizeInTiles();

    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            createTile(i, j, EntityTypes::ET_TILE);
        }
    }
}

void DemoWorldCreator::registerVillagerActions()
{
    CompResourceGatherer::gatheringActions = {{ResourceType::WOOD, UnitAction::CHOPPING},
                                              {ResourceType::STONE, UnitAction::MINING},
                                              {ResourceType::GOLD, UnitAction::MINING}};
    CompResourceGatherer::carryingActions = {{ResourceType::WOOD, UnitAction::CARRYING_LUMBER},
                                             {ResourceType::STONE, UnitAction::CARRYING_STONE},
                                             {ResourceType::GOLD, UnitAction::CARRYING_GOLD}};
}

uint32_t DemoWorldCreator::getResourceType(const std::string& resourceName)
{
    auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
    return typeRegistry->getEntityType(resourceName);
}
