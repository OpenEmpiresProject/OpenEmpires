#ifndef UNITCOMPONENTREFS_H
#define UNITCOMPONENTREFS_H

#include "CompMeleeAttack.h"
#include "CompRangeAttack.h"
#include "CompVision.h"
#include "StateManager.h"

#include <cstdint>
#include <tuple>

namespace core
{
class CompAction;
class CompAnimation;
class CompEntityInfo;
class CompPlayer;
class CompTransform;
class CompUnit;
class CompVision;

class UnitComponentRefs
{
  public:
    CompAction& action;
    CompAnimation& animation;
    CompEntityInfo& entityInfo;
    CompPlayer& player;
    CompTransform& transform;
    CompUnit& unit;
    CompVision& vision;
    CompMeleeAttack& meleeAttack;
    CompRangeAttack& rangeAttack;

    UnitComponentRefs(Ref<StateManager> stateMan, uint32_t entityID);

  private:
    UnitComponentRefs(std::tuple<CompAction&,
                                 CompAnimation&,
                                 CompEntityInfo&,
                                 CompPlayer&,
                                 CompTransform&,
                                 CompUnit&,
                                 CompVision&> components,
                      CompMeleeAttack& attack,
                      CompRangeAttack& rangeAttack);

    /*
     *   meleeAttack and rangeAttack are conditional, unlikely that a unit
     *   would have both. Yet this class should make command implementation
     *   easier by providing direct access to attack components. Hence,
     *   pointing to dummies when not available.
     */
    CompMeleeAttack m_dummyAttack;
    CompRangeAttack m_dummyRangeAttack;
};
} // namespace core

#endif