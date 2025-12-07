#ifndef UNITCOMPONENTREFS_H
#define UNITCOMPONENTREFS_H

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

    UnitComponentRefs(Ref<StateManager> stateMan, uint32_t entityID);

  private:
    UnitComponentRefs(std::tuple<CompAction&,
                                 CompAnimation&,
                                 CompEntityInfo&,
                                 CompPlayer&,
                                 CompTransform&,
                                 CompUnit&,
                                 CompVision&> components);
};
} // namespace core

#endif