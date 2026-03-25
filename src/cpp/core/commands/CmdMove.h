#ifndef CMDMOVE_H
#define CMDMOVE_H

#include "Feet.h"
#include "Path.h"
#include "PathService.h"
#include "Target.h"
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
    std::optional<Target> target;
    UnitAction actionOverride = UnitAction::MOVE;

  protected:
    void animate(int deltaTimeMs, int currentTick);
    bool move(int deltaTimeMs);
    virtual Feet avoidCollision(int deltaTimeMs);
    virtual bool isTargetCloseEnough() const;
    virtual bool isPositionCloseEnough(const Feet& pos) const;

    Path m_path;
    LazyServiceRef<Coordinates> m_coordinates;
    LazyServiceRef<Settings> m_settings;
    LazyServiceRef<PathService> m_pathService;

    // For handling frequent direction flips
    Feet m_previousBestDirection = Feet::zero;
    int m_numberOfDirectionFlips = 0;
    int m_directionFlipDurationMs = 0;
    const int DIRECTION_FLIP_THRESHOLD = 20;
    const int DIRECTION_FLIP_WAIT_TIME_MS = 2000;
    bool m_dontAnimate = false;

  private:
    void onStart() override;
    void onQueue() override;
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override;
    std::string toString() const override;
    Command* clone() override;
    void destroy() override;

    double distancePointToSegment(const Feet& p0, const Feet& p1, const Feet& q) const;
    void setPosition(const Feet& newPosFeet);
    bool hasLineOfSight(const Feet& target) const;
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
};
} // namespace core

#endif