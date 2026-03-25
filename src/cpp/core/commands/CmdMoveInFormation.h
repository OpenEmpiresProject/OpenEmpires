#ifndef CORE_CMDMOVEINFORMATION_H
#define CORE_CMDMOVEINFORMATION_H
#include "CmdMove.h"

namespace core
{
class CmdMoveInFormation : public CmdMove
{
  public:
    CmdMoveInFormation();
    ~CmdMoveInFormation();

  private:
    void onStart() override;
    void onQueue() override;
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override;
    std::string toString() const override;
    Command* clone() override;
    void destroy() override;
    bool move(int deltaTimeMs, const Feet& target);
    Feet getTargetPos() const;
    Feet avoidCollision(int deltaTimeMs) override;
    bool isTargetCloseEnough() const override;
    bool isPositionCloseEnough(const Feet& pos) const override;

    const int SLOT_REACHABLE_THRESHOLD_SQUARED =
        Constants::FEET_PER_TILE * Constants::FEET_PER_TILE;
    const int MOVING_TARGET_DISTANCE_THRESHOLD_SQUARED =
        Constants::FEET_PER_TILE * Constants::FEET_PER_TILE * 5 * 5;

    const int GOAL_RADIUS_OVERRIDE = 20;
    const int MOVEMENT_CHECK_INTERVAL_MS = 1000;
    // Cannot use a higher frequency (i.e. lower value) because depending on the unit speed
    // and goal radius, we need it to allow to move to next waypoint for real.
    const int PATH_REFRESH_FREQUENCY_IN_TICKS = 50;

    // Shadowing just to hide this. This command doesn't use this but formation
    Feet targetPos = Feet::null;

    Feet m_previousPos = Feet::zero;
    int m_timeSinceLastPreviousPosUpdateMs = 0;
    bool isUnitMoving(int deltaTimeMs);

    int m_currentTick = 0;
};
} // namespace core

#endif // CORE_CMDMOVEINFORMATION_H
