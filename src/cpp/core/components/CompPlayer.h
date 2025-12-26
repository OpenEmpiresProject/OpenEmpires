#ifndef COMPPLAYER_H
#define COMPPLAYER_H

#include "Player.h"

namespace core
{
class CompPlayer
{
  public:
    Ref<Player> player;

    static constexpr auto properties()
    {
        return std::tuple{/* No properties for now */};
    }
};

} // namespace core

#endif