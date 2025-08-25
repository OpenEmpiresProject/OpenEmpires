#include "GameShortcutResolver.h"

#include "GameTypes.h"

using namespace core;
using namespace game;

const ShortcutResolver::Action game::GameShortcutResolver::resolve(
    SDL_Scancode key, EntitySelectionData::Type currentSelection) const
{
    Action action;
    // TODO: Not accurate enough, need to check the entity type as well
    if (currentSelection == EntitySelectionData::Type::UNIT)
    {
        if (key == SDL_SCANCODE_M)
        {
            action.type = Action::Type::CREATE_BUILDING;
            action.entityType = EntityTypes::ET_MILL;
        }
        else if (key == SDL_SCANCODE_L)
        {
            action.type = Action::Type::CREATE_BUILDING;
            action.entityType = EntityTypes::ET_LUMBER_CAMP;
        }
        else if (key == SDL_SCANCODE_N)
        {
            action.type = Action::Type::CREATE_BUILDING;
            action.entityType = EntityTypes::ET_MINING_CAMP;
        }
        else if (key == SDL_SCANCODE_C)
        {
            action.type = Action::Type::CREATE_BUILDING;
            action.entityType = EntityTypes::ET_TOWN_CENTER;
        }
    }
    else if (currentSelection == EntitySelectionData::Type::BUILDING)
    {
        if (key == SDL_SCANCODE_V)
        {
            action.type = Action::Type::CREATE_UNIT;
            action.entityType = EntityTypes::ET_VILLAGER;
        }
    }
    return action;
}
