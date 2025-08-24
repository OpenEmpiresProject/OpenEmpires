#ifndef GAMESHORTCUTRESOLVER_H
#define GAMESHORTCUTRESOLVER_H

#include "ShortcutResolver.h"

namespace game
{
class GameShortcutResolver : public core::ShortcutResolver
{
  public:
    const Action resolve(SDL_Scancode key,
                         core::EntitySelectionData::Type currentSelection) const override;
};
} // namespace game

#endif
