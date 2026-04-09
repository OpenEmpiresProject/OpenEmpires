#include "BaseUnitFormation.h"

#include "PathService.h"
#include "ProximityChecker.h"
#include "Rect.h"

using namespace core;
//
// BaseUnitFormation::BaseUnitFormation()
// {
//     // constructor
// }
//
// BaseUnitFormation::~BaseUnitFormation()
// {
//     // destructor
// }

FormationSlot::FormationSlot(uint32_t entity, Ref<BaseUnitFormation> formation)
    : unitEntityId(entity), formation(formation)
{
}

uint32_t FormationSlot::getEntityId() const
{
    return unitEntityId;
}

core::Ref<core::BaseUnitFormation> FormationSlot::getFormation() const
{
    return formation;
}

bool FormationSlot::isValid() const
{
    return unitEntityId != entt::null and formation != nullptr;
}

bool FormationSlot::isInSameFormation(Ref<BaseUnitFormation> other) const
{
    return formation == other;
}

const core::Feet& BaseUnitFormation::getAnchor() const
{
    return m_anchor;
}

const std::vector<core::FormationSlot>& BaseUnitFormation::getSlots() const
{
    return m_slots;
}

void BaseUnitFormation::removeSlotFromFormation(FormationSlot& slot)
{
    for (auto it = m_slots.begin(); it != m_slots.end(); ++it)
    {
        if (it->getEntityId() == slot.getEntityId())
        {
            m_slots.erase(it);
            break;
        }
    }
    slot.removeFromFormation();
}

void BaseUnitFormation::removeAllSlots()
{
    for (auto& slot : m_slots)
    {
        slot.removeFromFormation();
    }
    m_slots.clear();
}

const core::Feet& BaseUnitFormation::getForward() const
{
    return m_forward;
}

core::FormationState BaseUnitFormation::getState() const
{
    return m_state;
}

core::Ref<core::Player> BaseUnitFormation::getControllingPlayer() const
{
    return m_controllingPlayer;
}

void BaseUnitFormation::giveUpControl()
{
    m_controllingPlayer = nullptr;
}

void BaseUnitFormation::updateTarget(const Feet& target)
{
    m_target = target;

    if (m_anchor != Feet::null and m_target != Feet::null and m_controllingPlayer != nullptr)
    {
        m_path = m_pathService->findPath(m_anchor, target, m_controllingPlayer);
    }
}

bool BaseUnitFormation::move(int deltaTimeMs)
{
    if (isTargetCloseEnough())
    {
        m_state = FormationState::REACHED;
        return true;
    }
    if (not m_path.isEmpty())
    {
        const auto& nextWaypoint = m_path.nextWaypoint();

        if (m_anchor.distanceSquared(nextWaypoint) < GOAL_RADIUS_SQ)
        {
            spdlog::debug("Formation next hop {} reached", nextWaypoint.toString());
            m_path.removeNextWaypoint();
        }
        else
        {
            auto timeS = (double) deltaTimeMs / 1000.0;

            m_forward = (nextWaypoint - m_anchor).normalized();
            m_anchor += (m_forward * (m_speed * timeS * m_settings->getGameSpeed()));
        }
    }
    bool reached = m_path.isEmpty();
    if (reached)
    {
        m_state = FormationState::REACHED;
    }
    return reached;
}

bool BaseUnitFormation::isTargetCloseEnough() const
{
    return m_anchor.distanceSquared(m_target) < GOAL_RADIUS_SQ;
}

void BaseUnitFormation::setControllingPlayer(Ref<Player> player)
{
    m_controllingPlayer = player;
}
