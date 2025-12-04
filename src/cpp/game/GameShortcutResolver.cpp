#include "GameShortcutResolver.h"

#include "EntityTypeRegistry.h"
#include "GameTypes.h"
#include "components/CompBuilder.h"
#include "components/CompUnitFactory.h"
#include "logging/Logger.h"

using namespace core;
using namespace game;

const ShortcutResolver::Action game::GameShortcutResolver::resolve(
    SDL_Scancode key, EntitySelectionData currentSelection) const
{
    char shortcutLetter = getLetter(key);
    if (shortcutLetter <= 0)
        return {};
    if (currentSelection.selection.selectedEntities.empty())
        return {};

    Action action;
    if (currentSelection.type == EntitySelectionData::Type::UNIT)
    {
        // TODO: Handle heterogeneous entity type selection
        auto firstEntity = currentSelection.selection.selectedEntities[0];
        auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();

        if (stateMan->hasComponent<CompBuilder>(firstEntity))
        {
            auto& builder = stateMan->getComponent<CompBuilder>(firstEntity);
            if (builder.buildableTypesByShortcut.value().contains(shortcutLetter))
            {
                uint32_t entityTypeToCreate =
                    builder.buildableTypesByShortcut.value().at(shortcutLetter);
                action.entityType = entityTypeToCreate;
                action.type = Action::Type::CREATE_BUILDING;
            }
        }

        // TODO: Need better shortcut handling
        if (shortcutLetter == 'g')
        {
            action.type = Action::Type::GARRISON;
        }
    }
    else if (currentSelection.type == EntitySelectionData::Type::BUILDING)
    {
        // TODO: Handle heterogeneous entity type selection
        auto firstEntity = currentSelection.selection.selectedEntities[0];
        auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();

        if (stateMan->hasComponent<CompUnitFactory>(firstEntity) == false)
        {
            spdlog::warn("Cannot resolve shortcut to create unit since selected entity "
                         "{} is not a unit factory",
                         firstEntity);
            return {};
        }

        auto& factory = stateMan->getComponent<CompUnitFactory>(firstEntity);
        if (factory.producibleUnitShortcuts.value().contains(shortcutLetter))
        {
            uint32_t entityTypeToCreate =
                factory.producibleUnitShortcuts.value().at(shortcutLetter);
            action.entityType = entityTypeToCreate;
            action.type = Action::Type::CREATE_UNIT;
        }

        if (shortcutLetter == 'u')
        {
            action.type = Action::Type::UNGARRISON;
        }
    }
    return action;
}

char GameShortcutResolver::getLetter(SDL_Scancode key) const
{
    if (key >= SDL_SCANCODE_A && key <= SDL_SCANCODE_Z)
    {
        return key - SDL_SCANCODE_A + 'a';
    }
    return 0;
}

SDL_Scancode GameShortcutResolver::getKeyCode(char letter) const
{
    if (letter >= 'a' && letter <= 'z')
    {
        char sdlCode = letter - 'a' + SDL_SCANCODE_A;
        return static_cast<SDL_Scancode>(sdlCode);
    }
    return SDL_Scancode::SDL_SCANCODE_UNKNOWN;
}
