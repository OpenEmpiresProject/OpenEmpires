#include "UnitComponentRefs.h"

#include "CompAction.h"
#include "CompAnimation.h"
#include "CompDirty.h"
#include "CompEntityInfo.h"
#include "CompGraphics.h"
#include "CompPlayer.h"
#include "CompTransform.h"
#include "CompUnit.h"

using namespace ion;

UnitComponentRefs::UnitComponentRefs(std::tuple<CompAction&,
                                                CompAnimation&,
                                                CompDirty&,
                                                CompEntityInfo&,
                                                CompPlayer&,
                                                CompTransform&,
                                                CompUnit&> components)
    : action{std::get<0>(components)}, animation{std::get<1>(components)},
      dirty{std::get<2>(components)}, entityInfo{std::get<3>(components)},
      player{std::get<4>(components)}, transform{std::get<5>(components)},
      unit{std::get<6>(components)}
{
}

UnitComponentRefs::UnitComponentRefs(Ref<GameState> gameState, uint32_t entityID)
    : UnitComponentRefs(gameState->getComponents<CompAction,
                                                 CompAnimation,
                                                 CompDirty,
                                                 CompEntityInfo,
                                                 CompPlayer,
                                                 CompTransform,
                                                 CompUnit>(entityID))
{
}