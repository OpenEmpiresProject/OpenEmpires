#include "UnitComponentRefs.h"

#include "CompAction.h"
#include "CompAnimation.h"
#include "CompEntityInfo.h"
#include "CompGraphics.h"
#include "CompPlayer.h"
#include "CompTransform.h"
#include "CompUnit.h"

using namespace core;

UnitComponentRefs::UnitComponentRefs(std::tuple<CompAction&,
                                                CompAnimation&,
                                                CompEntityInfo&,
                                                CompPlayer&,
                                                CompTransform&,
                                                CompUnit&,
                                                CompVision&> components)
    : action{std::get<0>(components)}, animation{std::get<1>(components)},
      entityInfo{std::get<2>(components)}, player{std::get<3>(components)},
      transform{std::get<4>(components)}, unit{std::get<5>(components)},
      vision{std::get<6>(components)}
{
}

UnitComponentRefs::UnitComponentRefs(Ref<StateManager> stateMan, uint32_t entityID)
    : UnitComponentRefs(stateMan->getComponents<CompAction,
                                                CompAnimation,
                                                CompEntityInfo,
                                                CompPlayer,
                                                CompTransform,
                                                CompUnit,
                                                CompVision>(entityID))
{
}