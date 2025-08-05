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

using namespace core;
using namespace game;

PlayerActionResolver::PlayerActionResolver(/* args */)
{
    registerCallback(Event::Type::KEY_UP, this, &PlayerActionResolver::onKeyUp);
    registerCallback(Event::Type::MOUSE_MOVE, this, &PlayerActionResolver::onMouseMove);
}

PlayerActionResolver::~PlayerActionResolver()
{
}

void PlayerActionResolver::onEvent(const core::Event& e)
{
}

void PlayerActionResolver::onMouseMove(const core::Event& e)
{
    m_lastMouseScreenPos = e.getData<MouseMoveData>().screenPos;
}

BuildingPlacementData createBuildingRequestData(uint32_t entityType, const Feet& pos)
{
    auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();

    BuildingPlacementData data;
    data.player = playerManager->getViewingPlayer();
    data.entityType = entityType;
    data.pos = pos;
    return data;
}

void PlayerActionResolver::onKeyUp(const core::Event& e)
{
    SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);
    auto coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
    auto worldPos = coordinates->screenUnitsToFeet(m_lastMouseScreenPos);

    if (scancode == SDL_SCANCODE_M)
    {
        auto data = createBuildingRequestData(EntityTypes::ET_MILL, worldPos);
        publishEvent(Event::Type::BUILDING_REQUESTED, data);
    }
    else if (scancode == SDL_SCANCODE_L)
    {
        auto data = createBuildingRequestData(EntityTypes::ET_LUMBER_CAMP, worldPos);
        publishEvent(Event::Type::BUILDING_REQUESTED, data);
    }
    else if (scancode == SDL_SCANCODE_N)
    {
        auto data = createBuildingRequestData(EntityTypes::ET_MINING_CAMP, worldPos);
        publishEvent(Event::Type::BUILDING_REQUESTED, data);
    }
}