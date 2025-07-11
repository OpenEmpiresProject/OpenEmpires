#ifndef UNITCOMPONENTREFS_H
#define UNITCOMPONENTREFS_H

#include "GameState.h"

#include <cstdint>
#include <tuple>

namespace ion
{
class CompAction;
class CompAnimation;
class CompDirty;
class CompEntityInfo;
class CompPlayer;
class CompTransform;
class CompUnit;

class UnitComponentRefs
{
  public:
    CompAction& action;
    CompAnimation& animation;
    CompDirty& dirty;
    CompEntityInfo& entityInfo;
    CompPlayer& player;
    CompTransform& transform;
    CompUnit& unit;

    UnitComponentRefs(Ref<GameState> gameState, uint32_t entityID);

  private:
    UnitComponentRefs(std::tuple<CompAction&,
                                 CompAnimation&,
                                 CompDirty&,
                                 CompEntityInfo&,
                                 CompPlayer&,
                                 CompTransform&,
                                 CompUnit&> components);
};
} // namespace ion

#endif