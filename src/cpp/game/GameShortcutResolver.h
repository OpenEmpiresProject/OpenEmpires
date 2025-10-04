#ifndef GAMESHORTCUTRESOLVER_H
#define GAMESHORTCUTRESOLVER_H

#include "GameTypes.h"
#include "ShortcutResolver.h"

namespace game
{
class GameShortcutResolver : public core::ShortcutResolver
{
  public:
    const Action resolve(SDL_Scancode key,
                         core::EntitySelectionData currentSelection) const override;

  private:
    const std::string m_villagerName = "villager";

  private:
    SDL_Scancode getKeyCode(char letter) const;
    char getLetter(SDL_Scancode key) const;
};
} // namespace game

#endif
