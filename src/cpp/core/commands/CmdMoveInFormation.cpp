#include "CmdMoveInFormation.h"

#include "ProximityChecker.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/ObjectPool.h"
using namespace core;

CmdMoveInFormation::CmdMoveInFormation()
{
    // constructor
}

CmdMoveInFormation::~CmdMoveInFormation()
{
    // destructor
}

void CmdMoveInFormation::onStart()
{
}

void CmdMoveInFormation::onQueue()
{
}

bool CmdMoveInFormation::onExecute(int deltaTimeMs,
                                   int currentTick,
                                   std::list<Command*>& subCommands)
{
    auto slotAbsolutePos = getTargetPos();
    m_currentTick = currentTick;

    if (slotAbsolutePos.isNull() == false) [[likely]]
    {
        animate(deltaTimeMs, currentTick);
        return move(deltaTimeMs, slotAbsolutePos);
    }
    else [[unlikely]]
    {
        spdlog::error("Slot position {} is not properly set to move.", slotAbsolutePos.toString());
        return true;
    }
}

std::string CmdMoveInFormation::toString() const
{
    return "move-in-formation";
}

Command* CmdMoveInFormation::clone()
{
    return ObjectPool<CmdMoveInFormation>::acquire(*this);
}

void CmdMoveInFormation::destroy()
{
    ObjectPool<CmdMoveInFormation>::release(this);
}

/*
 *   Approach:
 *   1. Find path to target. PathService will use long-distance path finding if required.
 *   2. Once the target is within certain threshold, keep using PathService to find renewed path.
 *       Threshold is essential to avoid long-distance expensive path searches.
 *   3. Move towards the next waypoint
 */
bool CmdMoveInFormation::move(int deltaTimeMs, const Feet& target)
{
    auto isStillInFormation = m_components->unit.formationSlot.isValid();
    if (not isStillInFormation)
    {
        // Unit is no longer in formation, this command is no longer valid.
        return true;
    }

    auto& passabilityMap = m_stateMan->getPassabilityMap();
    auto isBlockedAhead =
        not passabilityMap.isPassableFor(target.toTile(), m_components->player.player->getId());
    if (isBlockedAhead)
    {
        // Nothing we can do, the slot is blocked, let's wait and see.
        // TODO: Predict whether the slot will be unblocked soon and move there.
        m_dontAnimate = true; // Stand still instead of walking without moving
        return false;
    }
    auto isTimeToRefreshPath = m_currentTick % PATH_REFRESH_FREQUENCY_IN_TICKS == 0;

    // Unit's slot in the formation is always moving, therefore, we need to calculate the
    // path to the slot frequently.
    if (m_path.isEmpty() or isTimeToRefreshPath)
    {
        m_path = m_pathService->findPath(m_components->transform.position, target,
                                         m_components->player.player);
        if (m_path.getWaypoints().size() > 1)
        {
            spdlog::debug("Unit {}, path refreshed. Waypoints {}. Current pos {}", m_entityID,
                          m_path.getWaypoints().size(),
                          m_components->transform.position.toString());
        }
    }

    auto slotReached = CmdMove::move(deltaTimeMs);

    if (slotReached)
    {
        // No matter where the unit came from, once the slot is reached (regardless whether
        // it is temporarily of not), we want the unit to face towards the formation forward.
        const auto& formationForward =
            m_components->unit.formationSlot.getFormation()->getForward();
        m_components->transform.faceAtDirection(formationForward);

        spdlog::debug("Unit {} reached formation slot at {}", m_entityID,
                      m_components->transform.position.toString());
    }

    // Slot can be temporarily reached, but if the formation is still MOVING, we cannot
    // consider the command as completed.
    auto formationReachedTarget =
        m_components->unit.formationSlot.getFormation()->getState() == FormationState::REACHED;
    return slotReached and formationReachedTarget;
}

Feet CmdMoveInFormation::getTargetPos() const
{
    const auto& slot = m_components->unit.formationSlot;
    if (slot.isValid())
    {
        return slot.getFormation()->getAnchor() + slot.offsetFromAnchor;
    }

    // Should not happen, but to avoid crashing just use unit position which will make
    // the command to exit
    spdlog::warn("Unit's formation is invalid, falling back to current position");
    return m_components->transform.position;
}

core::Feet CmdMoveInFormation::avoidCollision(int deltaTimeMs, const Feet& goalPos)
{
    auto preferredDir = goalPos - m_components->transform.position;
    auto deltaTimeS = (double) deltaTimeMs / 1000.0;
    const auto& currPos = m_components->transform.position;
    auto speed = m_components->transform.speed;
    float lookAheadDuration = 0.5f;

    auto slotAbsolutePos = getTargetPos();
    target.emplace(slotAbsolutePos, Target::Type::POSITION);

    return m_pathService->getBestAvoidanceDirectionVector(
        currPos, preferredDir, GOAL_RADIUS_OVERRIDE, speed, lookAheadDuration, m_entityID,
        AvoidnaceQuality::HIGH, target.value());
}

bool CmdMoveInFormation::isPositionCloseEnough(const Feet& pos) const
{
    return ProximityChecker::isInProximity(m_components->transform.position, pos,
                                           GOAL_RADIUS_OVERRIDE);
}

bool CmdMoveInFormation::isUnitMoving(int deltaTimeMs)
{
    m_timeSinceLastPreviousPosUpdateMs += deltaTimeMs;
    if (m_timeSinceLastPreviousPosUpdateMs >= MOVEMENT_CHECK_INTERVAL_MS)
    {
        auto distanceTraveled = m_components->transform.position.distance(m_previousPos);
        auto distanceSupposedToTravel =
            m_components->transform.speed * (m_timeSinceLastPreviousPosUpdateMs / 1000.0f);

        // If unit hasn't traveled at least half of the distance it is supposed to travel,
        // we consider the unit is stuck.
        if (distanceTraveled < (distanceSupposedToTravel / 2))
        {
            spdlog::warn("Unit {} stuck", m_components->entityInfo.entityId.value());

            m_timeSinceLastPreviousPosUpdateMs = 0;
            m_previousPos = m_components->transform.position;

            return false;
        }

        m_timeSinceLastPreviousPosUpdateMs = 0;
        m_previousPos = m_components->transform.position;
    }
    return true;
}
