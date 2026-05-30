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
                                                CompVision&> components,
                                     CompMeleeAttack& attack,
                                     CompRangeAttack& rangeAttack)
    : action{std::get<0>(components)}, animation{std::get<1>(components)},
      entityInfo{std::get<2>(components)}, player{std::get<3>(components)},
      transform{std::get<4>(components)}, unit{std::get<5>(components)},
      vision{std::get<6>(components)}, meleeAttack(attack), rangeAttack(rangeAttack)
{
}

UnitComponentRefs::UnitComponentRefs(Ref<StateManager> stateMan, uint32_t entityID)
    : UnitComponentRefs(stateMan->getComponents<CompAction,
                                                CompAnimation,
                                                CompEntityInfo,
                                                CompPlayer,
                                                CompTransform,
                                                CompUnit,
                                                CompVision>(entityID),
                        (stateMan->hasComponent<CompMeleeAttack>(entityID)
                             ? stateMan->getComponent<CompMeleeAttack>(entityID)
                             : m_dummyAttack),
                        (stateMan->hasComponent<CompRangeAttack>(entityID)
                             ? stateMan->getComponent<CompRangeAttack>(entityID)
                             : m_dummyRangeAttack))
{
}