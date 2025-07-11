#include "PlayerActionResolver.h"

#include "Coordinates.h"
#include "Event.h"
#include "GameState.h"
#include "PlayerManager.h"
#include "ServiceRegistry.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"

#include <SDL3/SDL.h>

using namespace ion;
using namespace game;

PlayerActionResolver::PlayerActionResolver(/* args */)
{
    registerCallback(Event::Type::KEY_UP, this, &PlayerActionResolver::onKeyUp);
    registerCallback(Event::Type::MOUSE_MOVE, this, &PlayerActionResolver::onMouseMove);
}

PlayerActionResolver::~PlayerActionResolver()
{
}

void PlayerActionResolver::onEvent(const ion::Event& e)
{
}

void PlayerActionResolver::onMouseMove(const ion::Event& e)
{
    m_lastMouseScreenPos = e.getData<MouseMoveData>().screenPos;
}

BuildingPlacementData createBuildingRequestData(uint32_t entity)
{
    auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();

    BuildingPlacementData data;
    data.player = playerManager->getViewingPlayer();
    data.entity = entity;
    return data;
}

void PlayerActionResolver::onKeyUp(const ion::Event& e)
{
    SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);
    auto coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
    auto worldPos = coordinates->screenUnitsToFeet(m_lastMouseScreenPos);

    if (scancode == SDL_SCANCODE_B)
    {
        auto entity =
            createBuilding(worldPos, EntityTypes::ET_MILL, Size(2, 2), ResourceType::RT_NONE);
        auto data = createBuildingRequestData(entity);
        publishEvent(Event::Type::BUILDING_REQUESTED, data);
    }
    else if (scancode == SDL_SCANCODE_N)
    {
        auto entity = createBuilding(worldPos, EntityTypes::ET_MARKETPLACE, Size(4, 4),
                                     ResourceType::RT_NONE);
        auto data = createBuildingRequestData(entity);
        publishEvent(Event::Type::BUILDING_REQUESTED, data);
    }
    else if (scancode == SDL_SCANCODE_M)
    {
        auto entity =
            createBuilding(worldPos, EntityTypes::ET_LUMBER_CAMP, Size(2, 2), ResourceType::WOOD);
        auto data = createBuildingRequestData(entity);
        publishEvent(Event::Type::BUILDING_REQUESTED, data);
    }
    else if (scancode == SDL_SCANCODE_L)
    {
        auto entity = createBuilding(worldPos, EntityTypes::ET_MINING_CAMP, Size(2, 2),
                                     ResourceType::GOLD | ResourceType::STONE);
        auto data = createBuildingRequestData(entity);
        publishEvent(Event::Type::BUILDING_REQUESTED, data);
    }
}

uint32_t PlayerActionResolver::createBuilding(const Feet& targetFeetPos,
                                              EntityTypes buildingType,
                                              Size size,
                                              uint8_t resourceTypesAccept)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto entity = gameState->createEntity();
    auto transform = CompTransform(targetFeetPos);
    transform.face(Direction::NORTHWEST);
    gameState->addComponent(entity, transform);
    gameState->addComponent(entity, CompRendering());
    CompGraphics gc;
    gc.entityID = entity;
    gc.entityType = buildingType;
    gc.layer = GraphicLayer::ENTITIES;
    gameState->addComponent(entity, gc);
    gameState->addComponent(entity, CompEntityInfo(buildingType));

    auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
    auto player = playerManager->getViewingPlayer();
    gameState->addComponent(entity, CompPlayer{player});

    CompBuilding building;
    building.size = size;
    building.lineOfSight = 256 * 5;
    building.addDropOff(resourceTypesAccept);
    building.visualVariationByProgress = {{33, 1}, {66, 2}, {99, 3}, {100, 0}};
    gameState->addComponent(entity, building);

    // auto box = getBoundingBox(m_drs, 1254, 1);
    // CompSelectible sc;
    // sc.boundingBoxes[static_cast<int>(Direction::NONE)] = box;
    // sc.selectionIndicator = {
    //     GraphicAddon::Type::RHOMBUS,
    //     GraphicAddon::Rhombus{Constants::TILE_PIXEL_WIDTH, Constants::TILE_PIXEL_HEIGHT}};

    CompDirty dirty;
    dirty.markDirty(entity);
    gameState->addComponent(entity, dirty);
    return entity;
}