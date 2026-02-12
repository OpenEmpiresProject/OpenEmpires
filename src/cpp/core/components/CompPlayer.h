#ifndef COMPPLAYER_H
#define COMPPLAYER_H

#include "Player.h"
#include "debug.h"
#include "utils/Constants.h"

namespace core
{
class CompPlayer
{
  public:
    Ref<Player> player;
};

} // namespace core

#endif