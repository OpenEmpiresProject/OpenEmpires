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
using namespace ion;
using namespace drs;
using namespace std;

// TODO: This is a duplication of GraphicsLoaderFromDRS. Need to refactor to consolidate
shared_ptr<DRSFile> loadDRSFile(const string& drsFilename);
// frameId is 1 based but not 0 based
Rect<int> getBoundingBox(shared_ptr<DRSFile> drs, uint32_t slpId, uint32_t frameId);

DemoWorldCreator::DemoWorldCreator(std::stop_token* stopToken,
                                   std::shared_ptr<GameSettings> settings,
                                   GraphicsRegistry& graphicsRegistry,
                                   std::shared_ptr<Renderer> renderer,
                                   bool populateWorld)
    : SubSystem(stopToken), m_settings(std::move(settings)), m_graphicsRegistry(graphicsRegistry),
      m_renderer(std::move(renderer)), m_populateWorld(populateWorld)
{
}

bool DemoWorldCreator::isReady() const
{
    return m_isReady;
}

void DemoWorldCreator::loadEntities()
{
    spdlog::info("Loading entities...");

    m_drs = loadDRSFile("assets/graphics.drs");

    auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
    auto player = playerManager->createPlayer();

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
        generateMap(gameState->gameMap);
        createStoneOrGoldCluster(EntityTypes::ET_STONE, gameState->gameMap, 30, 30, 4);
        createStoneOrGoldCluster(EntityTypes::ET_GOLD, gameState->gameMap, 20, 30, 4);
        createVillager(player, Tile(25, 25));
        // createVillager(player, Tile(20, 20));
    }

    CompResourceGatherer::gatheringActions = {{ResourceType::WOOD, UnitAction::CHOPPING},
                                              {ResourceType::STONE, UnitAction::MINING},
                                              {ResourceType::GOLD, UnitAction::MINING}};
    CompResourceGatherer::carryingActions = {{ResourceType::WOOD, UnitAction::CARRYING_LUMBER},
                                             {ResourceType::STONE, UnitAction::CARRYING_STONE},
                                             {ResourceType::GOLD, UnitAction::CARRYING_GOLD}};

    GraphicsID resourcePanelBackground{
        .entityType = EntityTypes::ET_UI_ELEMENT,
        .entitySubType = EntitySubTypes::UI_WINDOW,
    };

    auto window = CreateRef<ui::Window>(resourcePanelBackground);
    window->name = "resourcePanel";
    ServiceRegistry::getInstance().getService<UIManager>()->registerWindow(window);

    GraphicsID woodAmountLabel{
        .entityType = EntityTypes::ET_UI_ELEMENT,
        .entitySubType = EntitySubTypes::UI_LABEL,
    };
    auto woodLabel = window->createChild<ui::Label>(woodAmountLabel);
    woodLabel->text = "0";
    woodLabel->rect = Rect<int>(35, 5, 50, 20);
    woodLabel->name = "wood";

    GraphicsID stoneAmountLabel{
        .entityType = EntityTypes::ET_UI_ELEMENT,
        .entitySubType = EntitySubTypes::UI_LABEL,
    };
    auto stoneLabel = window->createChild<ui::Label>(stoneAmountLabel);
    stoneLabel->text = "0";
    stoneLabel->rect = Rect<int>(265, 5, 50, 20);
    stoneLabel->name = "stone";

    GraphicsID goldAmountLabel{
        .entityType = EntityTypes::ET_UI_ELEMENT,
        .entitySubType = EntitySubTypes::UI_LABEL,
    };
    auto goldLabel = window->createChild<ui::Label>(goldAmountLabel);
    goldLabel->text = "0";
    goldLabel->rect = Rect<int>(195, 5, 50, 20);
    goldLabel->name = "gold";

    spdlog::info("Entity loading successfully.");
}

void DemoWorldCreator::createTree(TileMap& map, uint32_t x, uint32_t y)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();

    auto tree = gameState->createEntity();
    auto transform = CompTransform(x * 256 + 128, y * 256 + 128);
    gameState->addComponent(tree, transform);
    gameState->addComponent(tree, CompRendering());
    CompGraphics gc;
    gc.entityID = tree;
    gc.entityType = EntityTypes::ET_TREE;
    gc.entitySubType = 0; // 0=main tree, 1=chopped
    gc.layer = GraphicLayer::ENTITIES;

    gameState->addComponent(tree, gc);
    // TODO: Should not hard code
    CompEntityInfo entityInfo(EntityTypes::ET_TREE, 0, rand() % 10);
    entityInfo.entityId = tree;
    gameState->addComponent(tree, entityInfo);
    gameState->addComponent(tree, CompDirty());

    // TODO: This doesn't work. Need to conslidate resource and graphic loading and handle this
    auto box = getBoundingBox(m_drs, 1254, 1);
    CompSelectible sc;
    sc.boundingBoxes[static_cast<int>(Direction::NONE)] = box;
    sc.selectionIndicator = {
        GraphicAddon::Type::RHOMBUS,
        GraphicAddon::Rhombus{Constants::TILE_PIXEL_WIDTH, Constants::TILE_PIXEL_HEIGHT}};

    gameState->addComponent(tree, sc);
    CompResource compRes;
    PropertyInitializer::set(compRes.original, Resource(ResourceType::WOOD, 100));
    gameState->addComponent(tree, compRes);

    map.addEntity(MapLayerType::STATIC, Tile(x, y), tree);
}

void DemoWorldCreator::createStoneOrGold(EntityTypes entityType,
                                         ion::TileMap& gameMap,
                                         uint32_t x,
                                         uint32_t y)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();

    auto stone = gameState->createEntity();
    auto transform = CompTransform(x * 256 + 128, y * 256 + 128);
    gameState->addComponent(stone, transform);
    gameState->addComponent(stone, CompRendering());
    CompGraphics gc;
    gc.entityID = stone;
    gc.entityType = entityType;
    gc.entitySubType = 0;
    gc.layer = GraphicLayer::ENTITIES;

    gameState->addComponent(stone, gc);
    CompEntityInfo entityInfo(entityType, 0, rand() % 7);
    entityInfo.entityId = stone;
    gameState->addComponent(stone, entityInfo);
    gameState->addComponent(stone, CompDirty());

    // TODO: This doesn't work. Need to conslidate resource and graphic loading and handle this
    auto box = getBoundingBox(m_drs, 1034, 1);
    CompSelectible sc;
    sc.boundingBoxes[static_cast<int>(Direction::NONE)] = box;
    sc.selectionIndicator = {
        GraphicAddon::Type::RHOMBUS,
        GraphicAddon::Rhombus{Constants::TILE_PIXEL_WIDTH, Constants::TILE_PIXEL_HEIGHT}};

    gameState->addComponent(stone, sc);

    ResourceType resourceType = ResourceType::STONE;
    if (entityType == EntityTypes::ET_GOLD)
        resourceType = ResourceType::GOLD;

    CompResource compRes;
    PropertyInitializer::set(compRes.original, Resource(resourceType, 1000));
    gameState->addComponent(stone, compRes);

    gameMap.addEntity(MapLayerType::STATIC, Tile(x, y), stone);
}

void DemoWorldCreator::createVillager(Ref<ion::Player> player, const Tile& tilePos)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto villager = factory->createEntity(EntityTypes::ET_VILLAGER);
    auto [transform, unit, selectible, playerComp] =
        gameState->getComponents<CompTransform, CompUnit, CompSelectible, CompPlayer>(villager);

    transform.position = Feet(tilePos.x * 256 + 128, tilePos.x * 256 + 50);
    transform.face(Direction::SOUTH);
    auto box = getBoundingBox(m_drs, 1388, 1);
    selectible.boundingBoxes[static_cast<int>(Direction::NONE)] = box;
    selectible.selectionIndicator = {GraphicAddon::Type::ISO_CIRCLE,
                                     GraphicAddon::IsoCircle{10, Vec2(0, 0)}};
    playerComp.player = player;

    auto newTile = transform.position.toTile();
    gameState->gameMap.addEntity(MapLayerType::UNITS, newTile, villager);

    player->getFogOfWar()->markAsExplored(transform.position, unit.lineOfSight);
}

void DemoWorldCreator::createStoneOrGoldCluster(
    EntityTypes entityType, ion::TileMap& gameMap, uint32_t xHint, uint32_t yHint, uint8_t amount)
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
                                  ion::Ref<ion::GameState> gameState,
                                  EntityTypes entityType)
{
    auto size = m_settings->getWorldSizeInTiles();

    auto tile = gameState->createEntity();
    gameState->addComponent(tile, CompTransform(x * 256, y * 256));
    gameState->addComponent(tile, CompRendering());
    gameState->addComponent(tile, CompDirty());

    CompGraphics gc;
    gc.entityID = tile;
    gc.entityType = entityType;
    // tc = sqrt(total_tile_frame_count)
    int tc = 10;
    // Convet our top corner based coordinate to left corner based coordinate
    int newX = size.height - y;
    int newY = x;
    // AOE2 standard tiling rule. From OpenAge documentation
    int tileVariation = (newX % tc) + ((newY % tc) * tc) + 1;

    CompEntityInfo entityInfo(entityType, 0, tileVariation);
    entityInfo.entityId = tile;
    gameState->addComponent(tile, entityInfo);

    DebugOverlay overlay{DebugOverlay::Type::RHOMBUS, ion::Color::GREY,
                         DebugOverlay::FixedPosition::BOTTOM_CENTER};
    overlay.customPos1 = DebugOverlay::FixedPosition::CENTER_LEFT;
    overlay.customPos2 = DebugOverlay::FixedPosition::CENTER_RIGHT;
    gc.debugOverlays.push_back(overlay);
    gc.layer = GraphicLayer::GROUND;

    auto map = gameState->gameMap;
    map.addEntity(MapLayerType::GROUND, Tile(x, y), tile);
    gameState->addComponent(tile, gc);
}

void DemoWorldCreator::init()
{
    Resource::registerResourceType(ResourceType::WOOD);
    Resource::registerResourceType(ResourceType::STONE);
    Resource::registerResourceType(ResourceType::GOLD);
    loadEntities();
    m_isReady = true;
}

void DemoWorldCreator::shutdown()
{
    // Cleanup code for resource loading
}

// shared_ptr<DRSFile> loadDRSFile2(const string& drsFilename)
// {
//     auto drs = make_shared<DRSFile>();
//     if (!drs->load(drsFilename))
//     {
//         throw runtime_error("Failed to load DRS file: " + drsFilename);
//     }
//     return std::move(drs);
// }

Rect<int> getBoundingBox(shared_ptr<DRSFile> drs, uint32_t slpId, uint32_t frameId)
{
    auto frameInfos = drs->getSLPFile(slpId).getFrameInfos();
    auto frame = frameInfos[frameId - 1];
    Rect<int> box(frame.hotspot_x, frame.hotspot_y, frame.width, frame.height);
    return box;
}