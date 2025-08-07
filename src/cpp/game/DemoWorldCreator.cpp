#include "DemoWorldCreator.h"

#include "Coordinates.h"
#include "EntityFactory.h"
#include "GameState.h"
#include "GameTypes.h"
#include "PlayerManager.h"
#include "Renderer.h"
#include "ServiceRegistry.h"
#include "SubSystemRegistry.h"
#include "Tile.h"
#include "UI.h"
#include "UIManager.h"
#include "commands/CmdIdle.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"
#include "utils/Types.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <memory>
#include <random>
#include <string>

namespace fs = std::filesystem;
using namespace game;
using namespace core;
using namespace drs;
using namespace std;

DemoWorldCreator::DemoWorldCreator(std::stop_token* stopToken,
                                   std::shared_ptr<GameSettings> settings,
                                   bool populateWorld)
    : SubSystem(stopToken), m_settings(std::move(settings)), m_populateWorld(populateWorld)
{
}

bool DemoWorldCreator::isReady() const
{
    return m_isReady;
}

void DemoWorldCreator::loadEntities()
{
    spdlog::info("Loading entities...");

    auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
    auto player = playerManager->createPlayer();
    auto player2 = playerManager->createPlayer();

    auto gameState = ServiceRegistry::getInstance().getService<GameState>();

    auto size = m_settings->getWorldSizeInTiles();

    for (size_t i = 0; i < size.width; i++)
    {
        for (size_t j = 0; j < size.height; j++)
        {
            createTile(i, j, gameState, EntityTypes::ET_TILE);
        }
    }

    if (m_populateWorld)
    {
        generateMap(gameState->gameMap());
        createStoneOrGoldCluster(EntityTypes::ET_STONE, gameState->gameMap(), 30, 30, 4);
        createStoneOrGoldCluster(EntityTypes::ET_GOLD, gameState->gameMap(), 20, 30, 4);
        createVillager(player2, Tile(25, 25));
        createVillager(player, Tile(20, 20));
    }

    CompResourceGatherer::gatheringActions = {{ResourceType::WOOD, UnitAction::CHOPPING},
                                              {ResourceType::STONE, UnitAction::MINING},
                                              {ResourceType::GOLD, UnitAction::MINING}};
    CompResourceGatherer::carryingActions = {{ResourceType::WOOD, UnitAction::CARRYING_LUMBER},
                                             {ResourceType::STONE, UnitAction::CARRYING_STONE},
                                             {ResourceType::GOLD, UnitAction::CARRYING_GOLD}};

    ui::Widget::s_entityType = EntityTypes::ET_UI_ELEMENT;
    ui::Widget::s_entitySubType = EntitySubTypes::EST_DEFAULT;
    GraphicsID resourcePanelBackground{.entityType = EntityTypes::ET_UI_ELEMENT,
                                       .entitySubType = EntitySubTypes::EST_UI_RESOURCE_PANEL};
    auto window = CreateRef<ui::Window>();
    window->setName("resourcePanel");
    window->setBackgroundImage(resourcePanelBackground.hash());
    ServiceRegistry::getInstance().getService<UIManager>()->registerWindow(window);

    auto woodLabel = window->createChild<ui::Label>();
    woodLabel->setText("0");
    woodLabel->setRect(Rect<int>(35, 5, 50, 20));
    woodLabel->setName("wood");

    auto stoneLabel = window->createChild<ui::Label>();
    stoneLabel->setText("0");
    stoneLabel->setRect(Rect<int>(265, 5, 50, 20));
    stoneLabel->setName("stone");

    auto goldLabel = window->createChild<ui::Label>();
    goldLabel->setText("0");
    goldLabel->setRect(Rect<int>(195, 5, 50, 20));
    goldLabel->setName("gold");

    auto playerIdLabel = window->createChild<ui::Label>();
    playerIdLabel->setRect(Rect<int>(420, 5, 50, 20));
    playerIdLabel->setName("player");
    playerIdLabel->setTextColor(core::Color::WHITE);

    GraphicsID controlPanelBackground{.entityType = EntityTypes::ET_UI_ELEMENT,
                                      .entitySubType = EntitySubTypes::EST_UI_CONTROL_PANEL};
    auto controlPanel = CreateRef<ui::Window>();
    controlPanel->setName("controlPanel");
    controlPanel->setBackgroundImage(controlPanelBackground.hash());
    controlPanel->setRect(Rect<int>(0, -1, 506, 145)); // -1 is to attach to bottom of screen
    ServiceRegistry::getInstance().getService<UIManager>()->registerWindow(controlPanel);

    auto hLayout = controlPanel->createChild<ui::Layout>();

    auto commandsLayout = hLayout->createChild<ui::Layout>();
    commandsLayout->setRect(Rect<int>(0, 0, 210, 145)); // TODO: Can we simplify this?

    auto infoLayout = hLayout->createChild<ui::Layout>();
    infoLayout->setRect(Rect<int>(0, 0, 290, 145)); // TODO: Can we simplify this?
    infoLayout->setMargin(20);
    infoLayout->setSpacing(10);

    auto selectedIcon = infoLayout->createChild<ui::Label>();
    selectedIcon->setRect(Rect<int>(0, 0, 50, 50));
    selectedIcon->setName("selected_icon");
    selectedIcon->setVisible(false);

    spdlog::info("Entity loading successfully.");
}

void DemoWorldCreator::createTree(TileMap& map, uint32_t x, uint32_t y)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto tree = factory->createEntity(EntityTypes::ET_TREE, 0);
    auto [transform, selectible, info] =
        gameState->getComponents<CompTransform, CompSelectible, CompEntityInfo>(tree);

    int variation = rand() % 10;

    transform.position = Feet(x * 256 + 128, y * 256 + 128);
    info.variation = variation;

    map.addEntity(MapLayerType::STATIC, Tile(x, y), tree);

    // Add shadow
    auto shadow = factory->createEntity(EntityTypes::ET_TREE, EntitySubTypes::EST_TREE_SHADOW);
    auto [transformShadow, infoShadow] =
        gameState->getComponents<CompTransform, CompEntityInfo>(shadow);

    infoShadow.variation = variation;
    transformShadow.position = Feet(x * 256 + 128, y * 256 + 128);

    map.addEntity(MapLayerType::ON_GROUND, Tile(x, y), shadow);
}

void DemoWorldCreator::createStoneOrGold(EntityTypes entityType,
                                         core::TileMap& gameMap,
                                         uint32_t x,
                                         uint32_t y)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto entity = factory->createEntity(entityType, 0);
    auto [transform, selectible, info] =
        gameState->getComponents<CompTransform, CompSelectible, CompEntityInfo>(entity);

    transform.position = Feet(x * 256 + 128, y * 256 + 128);
    info.variation = rand() % 7;

    gameMap.addEntity(MapLayerType::STATIC, Tile(x, y), entity);
}

void DemoWorldCreator::createVillager(Ref<core::Player> player, const Tile& tilePos)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto villager = factory->createEntity(EntityTypes::ET_VILLAGER, 0);
    auto [transform, unit, selectible, playerComp] =
        gameState->getComponents<CompTransform, CompUnit, CompSelectible, CompPlayer>(villager);

    transform.position = Feet(tilePos.x * 256 + 128, tilePos.x * 256 + 50);
    transform.face(Direction::SOUTH);
    selectible.selectionIndicator = {GraphicAddon::Type::ISO_CIRCLE,
                                     GraphicAddon::IsoCircle{10, Vec2(0, 0)}};
    playerComp.player = player;

    auto newTile = transform.position.toTile();
    gameState->gameMap().addEntity(MapLayerType::UNITS, newTile, villager);

    player->getFogOfWar()->markAsExplored(transform.position, unit.lineOfSight);
}

void DemoWorldCreator::createStoneOrGoldCluster(
    EntityTypes entityType, core::TileMap& gameMap, uint32_t xHint, uint32_t yHint, uint8_t amount)
{
    if (amount == 0)
        return;

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
        createStoneOrGold(entityType, gameMap, cx, cy);
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

void DemoWorldCreator::generateMap(TileMap& gameMap)
{
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
                    createTree(gameMap, x, y);
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
            createTree(gameMap, x, y);
            ++treesPlaced;
        }
    }
}

void DemoWorldCreator::createTile(uint32_t x,
                                  uint32_t y,
                                  core::Ref<core::GameState> gameState,
                                  EntityTypes entityType)
{
    auto size = m_settings->getWorldSizeInTiles();

    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto entity = factory->createEntity(entityType, 0);
    auto [transform, info] = gameState->getComponents<CompTransform, CompEntityInfo>(entity);

    int tc = 10;
    // Convet our top corner based coordinate to left corner based coordinate
    int newX = size.height - y;
    int newY = x;
    // AOE2 standard tiling rule. From OpenAge documentation
    int tileVariation = (newX % tc) + ((newY % tc) * tc) + 1;

    transform.position = Feet(x * 256, y * 256);
    info.variation = tileVariation;

    gameState->gameMap().addEntity(MapLayerType::GROUND, Tile(x, y), entity);
}

void DemoWorldCreator::init()
{
    InGameResource::registerResourceType(ResourceType::WOOD);
    InGameResource::registerResourceType(ResourceType::STONE);
    InGameResource::registerResourceType(ResourceType::GOLD);
    loadEntities();
    m_isReady = true;
}

void DemoWorldCreator::shutdown()
{
    // Cleanup code for resource loading
}