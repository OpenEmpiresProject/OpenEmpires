#ifndef CMDMOVE_H
#define CMDMOVE_H

#include "Feet.h"
#include "commands/Command.h"
#include "utils/LazyServiceRef.h"

#include <entt/entity/registry.hpp>
#include <list>

namespace core
{
class Player;
template <typename T> class Rect;
class CompAction;
class CompAnimation;
class CompTransform;
class Coordinates;
class Settings;

class CmdMove : public Command
{
  public:
    Feet targetPos = Feet::null;
    uint32_t targetEntity = entt::null;
    UnitAction actionOverride = UnitAction::MOVE;

  private:
    std::list<Feet> m_path;
    LazyServiceRef<Coordinates> m_coordinates;
    LazyServiceRef<Settings> m_settings;
    Feet m_nextIntermediateGoal = Feet::null;

  private:
    void onStart() override;
    void onQueue() override;
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override;
    std::string toString() const override;
    Command* clone() override;
    void destroy() override;

    void animate(int deltaTimeMs, int currentTick);

    bool move(int deltaTimeMs);
    std::list<Feet> findPath(const Feet& endPosInFeet);
    Feet resolveCollision();
    Feet avoidCollision();
    double distancePointToSegment(const Feet& p0, const Feet& p1, const Feet& q) const;
    void setPosition(const Feet& newPosFeet);
    bool hasLineOfSight(const Feet& target) const;
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
                                       const Rect<float>& land) const;
    bool overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect) const;
    bool overlaps(const Feet& unitPos, float radiusSq, const Feet& targetPos) const;
    bool isTargetCloseEnough() const;
};
} // namespace core

#endif