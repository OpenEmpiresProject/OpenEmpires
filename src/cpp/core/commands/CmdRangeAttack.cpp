#include "CmdRangeAttack.h"

#include "CmdMove.h"
#include "EventPublisher.h"
#include "ProximityChecker.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompHealth.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/ObjectPool.h"

using namespace core;

void CmdRangeAttack::onStart()
{
    timeSinceLastAnimationEndMs = 0;
    m_components->unit.formationSlot = FormationSlot();
    m_components->animation.frame = 0;
}

void CmdRangeAttack::onQueue()
{
}

bool CmdRangeAttack::onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands)
{
    if (isCloseEnough())
    {
        animate(deltaTimeMs, currentTick);
        attack(deltaTimeMs);
    }
    else
    {
        moveCloser(subCommands);
    }

    bool isCompleted = isComplete();
    if (isCompleted)
    {
        // TODO: Implement
        /*if (not lookForAnotherToAttack(subCommands))
        {
            lookForAnotherToAttack(subCommands);
        }*/
    }

    return isCompleted;
}

std::string CmdRangeAttack::toString() const
{
    return "range-attack";
}

Command* CmdRangeAttack::clone()
{
    return ObjectPool<CmdRangeAttack>::acquire(*this);
}

void CmdRangeAttack::destroy()
{
    ObjectPool<CmdRangeAttack>::release(this);
}

void CmdRangeAttack::attack(int deltaTimeMs)
{
    if (createProjectile)
    {
        auto& targetTransform = m_stateMan->getComponent<CompTransform>(target);

        auto& projectile = m_components->rangeAttack.primaryProjectile.value();

        auto projectileEntityType = m_components->rangeAttack.projectileEntityType;
        auto projectileEntity = m_entityFactory->createEntity(projectileEntityType);
        auto releaseHeight = m_components->rangeAttack.projectileReleaseHeight;
        ProjectileData data(projectileEntity, m_components->transform.position,
                            targetTransform.position, projectile,
                            m_components->rangeAttack.projectileSpeed, releaseHeight);

        publishEvent(Event::Type::PROJECTILE_CREATED, data);

        createProjectile = false;
    }
}

void CmdRangeAttack::animate(int deltaTimeMs, int currentTick)
{
    // TODO: Incorporate attack delay

    m_components->action.action = UnitAction::ATTACK;
    auto& actionAnimation = m_components->animation.animations[m_components->action.action];

    auto ticksPerFrame =
        int(m_settings->getTicksPerSecond() / (actionAnimation.speed * m_settings->getGameSpeed()));
    auto shouldChangeFrame = currentTick % ticksPerFrame == 0;

    // If the animation is finished, wait reloadTime
    if (m_components->animation.frame >= (actionAnimation.frames - 1))
    {
        auto& projectile = m_components->rangeAttack.primaryProjectile.value();
        // TODO: Incorporate game speed
        const auto reloadTimeMs = projectile.reloadTimeS * 1000;
        timeSinceLastAnimationEndMs += deltaTimeMs;

        if (timeSinceLastAnimationEndMs < reloadTimeMs)
            return;

        timeSinceLastAnimationEndMs = 0;

        /*
         *   Regardless whether this is the tick to change the frame or not based
         *   on tickerPerFrame, we have waited more than that with reloadTime, so
         *   let's change the frame anyway now.
         */
        shouldChangeFrame = true;
    }

    if (shouldChangeFrame)
    {
        // Facing towards target only at the start of the animation is enough
        if (m_components->animation.frame == 0)
        {
            auto& targetTransform = m_stateMan->getComponent<CompTransform>(target);
            m_components->transform.face(targetTransform.position);
        }

        if (m_components->animation.frame == m_components->rangeAttack.projectileReleaseFrame)
            createProjectile = true;

        StateManager::markDirty(m_entityID);
        m_components->animation.frame++;
        m_components->animation.frame %= actionAnimation.frames; // Attacking is repeatable
    }
}

bool CmdRangeAttack::isCloseEnough()
{
    return ProximityChecker::isInProximity(m_components->transform.position,
                                           m_components->rangeAttack.attackRange, target,
                                           m_stateMan.getRef());
}

bool CmdRangeAttack::isComplete()
{
    // TODO - Perhaps there are other conditions for completion, such as target being destroyed
    // or no longer valid (e.g. out of LOS)
    auto& targetHealth = m_stateMan->getComponent<CompHealth>(target);
    return targetHealth.health <= 0;
}

void CmdRangeAttack::moveCloser(std::list<Command*>& newCommands)
{
    const auto& targetPosition = m_stateMan->getComponent<CompTransform>(target).position;

    spdlog::debug("Target {} at {} (tile {}) is not close enough to attack, moving...", target,
                  targetPosition.toString(), targetPosition.toTile().toString());
    auto moveCmd = ObjectPool<CmdMove>::acquire();
    moveCmd->collisionRadius = m_components->rangeAttack.attackRange;
    moveCmd->target.emplace(target);
    moveCmd->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
    newCommands.push_back(moveCmd);
}
