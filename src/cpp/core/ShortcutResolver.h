#ifndef SHORTCUTRESOLVER_H
#define SHORTCUTRESOLVER_H

#include "Event.h"

#include <SDL3/SDL.h>
#include <entt/entity/registry.hpp>

namespace core
{
class ShortcutResolver
{
  public:
    struct Action
    {
        enum class Type
        {
            INVALID,
            CREATE_BUILDING,
            CREATE_UNIT,
            UNIT_COMMAND
        };

        Type type = Type::INVALID;
        uint32_t entityType = entt::null;
    };

    virtual const Action resolve(SDL_Scancode key, EntitySelectionData currentSelection) const = 0;
    // TODO: handle other shortcut types including global shortcuts
};
} // namespace core

#endif
