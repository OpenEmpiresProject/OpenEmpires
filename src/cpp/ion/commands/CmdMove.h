#ifndef CMDMOVE_H
#define CMDMOVE_H

#include "Coordinates.h"
#include "Feet.h"
#include "Player.h"
#include "Rect.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompTransform.h"

#include <entt/entity/registry.hpp>
#include <list>

namespace ion
{
class CmdMove : public Command
{
  public:
    Feet targetPos = Feet::null;
    uint32_t targetEntity = entt::null;
    UnitAction actionOverride = UnitAction::MOVE;

  private:
    std::list<Feet> path;
    Ref<Coordinates> coordinates;
    Ref<Player> player;
    Feet nextIntermediateGoal = Feet::null;

    void onStart() override;
    void onQueue() override;
    bool onExecute(int deltaTimeMs, std::list<Command*>& subCommands) override;
    std::string toString() const override;
    void destroy() override;

    void animate(CompAction& action,
                 CompAnimation& animation,
                 CompDirty& dirty,
                 int deltaTimeMs,
                 uint32_t entityId);

    bool move(CompTransform& transform, int deltaTimeMs);
    std::list<Feet> findPath(const Feet& endPosInFeet, uint32_t entity);
    Feet resolveCollision(CompTransform& transform);
    Feet avoidCollision(CompTransform& transform);
    double distancePointToSegment(const Feet& p0, const Feet& p1, const Feet& q) const;
    Feet calculateNewPosition(CompTransform& transform, int timeMs);
    void setPosition(CompTransform& transform, const Feet& newPosFeet);
    bool hasLineOfSight(const Feet& target);
    void refinePath();
    uint32_t intersectsUnits(uint32_t self,
                             CompTransform& transform,
                             const Feet& start,
                             const Feet& end) const;
    bool lineIntersectsCircle(const Vec2& p1,
                              const Vec2& p2,
                              const Vec2& center,
                              float radius) const;
    Feet findClosestEdgeOfStaticEntity(uint32_t staticEntity,
                                       const Feet& fromPos,
                                       const Rect<float>& land);
    bool overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect);
    bool isTargetCloseEnough();
};
} // namespace ion

#endif