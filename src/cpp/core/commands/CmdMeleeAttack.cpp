#include "commands/CmdMeleeAttack.h"

#include "ProximityChecker.h"
#include "components/CompUnit.h"

void core::CmdMeleeAttack::onStart()
{
    timeSinceLastAttackMs = 0;
    m_components->unit.formationSlot = FormationSlot();
}

void core::CmdMeleeAttack::onQueue()
{
    // TODO: Reset frame to zero (since this is a new command)
}

bool core::CmdMeleeAttack::onExecute(int deltaTimeMs,
                                     int currentTick,
                                     std::list<Command*>& subCommands)
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
        /*if (not lookForAnotherToAttack(subCommands))
        {
            lookForAnotherToAttack(subCommands);
        }*/
    }

    return isCompleted;
}

std::string core::CmdMeleeAttack::toString() const
{
    return "attack";
}

core::Command* core::CmdMeleeAttack::clone()
{
    return ObjectPool<CmdMeleeAttack>::acquire(*this);
}

void core::CmdMeleeAttack::destroy()
{
    ObjectPool<CmdMeleeAttack>::release(this);
}

void core::CmdMeleeAttack::attack(int deltaTimeMs)
{
    const auto reloadTimeMs =
        1000.0f / (m_components->meleeAttack.attackRate * m_settings->getGameSpeed());
    timeSinceLastAttackMs += deltaTimeMs;

    if (timeSinceLastAttackMs >= reloadTimeMs)
    {
        timeSinceLastAttackMs = 0;

        auto [targetArmor, targetHealth, targetTransform] =
            m_stateMan->getComponents<CompArmor, CompHealth, CompTransform>(target);
        auto damage = getDamage(targetArmor);

        targetHealth.health -= damage;
        StateManager::markDirty(target);

        m_components->transform.face(targetTransform.position);

        spdlog::debug("Attacked target {} for {} damage. Target health is now {}/{}", target,
                      damage, targetHealth.health, targetHealth.maxHealth.value());
    }
}

void core::CmdMeleeAttack::animate(int deltaTimeMs, int currentTick)
{
    m_components->action.action = UnitAction::ATTACK;
    auto& actionAnimation = m_components->animation.animations[m_components->action.action];

    auto ticksPerFrame =
        int(m_settings->getTicksPerSecond() / (actionAnimation.speed * m_settings->getGameSpeed()));
    if (currentTick % ticksPerFrame == 0)
    {
        StateManager::markDirty(m_entityID);
        m_components->animation.frame++;
        m_components->animation.frame %= actionAnimation.frames; // Attacking is repeatable
    }
}

bool core::CmdMeleeAttack::isCloseEnough()
{
    debug_assert(target != entt::null, "Proposed entity to attack is null");

    return ProximityChecker::isInProximity(m_components->transform, target, m_stateMan.getRef());
}

bool core::CmdMeleeAttack::isComplete()
{
    // TODO - Perhaps there are other conditions for completion, such as target being destroyed
    // or no longer valid (e.g. out of LOS)
    auto& targetHealth = m_stateMan->getComponent<CompHealth>(target);
    return targetHealth.health <= 0;
}

void core::CmdMeleeAttack::moveCloser(std::list<Command*>& newCommands)
{
    debug_assert(target != entt::null, "Proposed entity to attack is null");

    const auto& targetPosition = m_stateMan->getComponent<CompTransform>(target).position;

    spdlog::debug("Target {} at {} (tile {}) is not close enough to attack, moving...", target,
                  targetPosition.toString(), targetPosition.toTile().toString());
    auto moveCmd = ObjectPool<CmdMove>::acquire();
    moveCmd->collisionRadius = m_components->transform.collisionRadius;
    moveCmd->target.emplace(target);
    moveCmd->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
    newCommands.push_back(moveCmd);
}

float core::CmdMeleeAttack::getDamage(const CompArmor& target) const
{
    float totalDamage = 0.0f;
    for (size_t i = 0; i < m_components->meleeAttack.attackPerClass.value().size(); ++i)
    {
        auto multipliedAttack = m_components->meleeAttack.attackPerClass[i] *
                                m_components->meleeAttack.attackMultiplierPerClass[i];
        auto damage = std::max(0.0f, multipliedAttack - target.armorPerClass[i]);
        totalDamage += damage;
    }
    totalDamage = std::max(1.0f, totalDamage * (1.0f - target.damageResistance.value()));
    return totalDamage;
}
