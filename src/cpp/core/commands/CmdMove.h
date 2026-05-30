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
    // TODO: This should be deprecated in favour of Target's arrival
    // evaluator capability now.
    // Move command relies on externally provided collision radius
    // instead of using any built-int unit radius to support custom
    // stopping distances. Will be used in situations like ranged
    // attacks.
    float collisionRadius = std::numeric_limits<float>::max();

  protected:
    void animate(int deltaTimeMs, int currentTick);
    bool move(int deltaTimeMs);
    virtual Feet avoidCollision(int deltaTimeMs, const Feet& goalPos);
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

    void updateUnitPosition(int deltaTimeMs, const Feet& forwardDir);
    bool stayIdleIfSeemsStuck(int deltaTimeMs, const Feet& forwardDir);
    void updateDebugOverlays(const Feet& forwardDirection);
};
} // namespace core

#endif