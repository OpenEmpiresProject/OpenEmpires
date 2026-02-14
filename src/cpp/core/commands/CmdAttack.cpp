#include "commands/CmdAttack.h"
#include "ProximityChecker.h"

void core::CmdAttack::onStart()
{
    timeSinceLastAttackMs = 0;
}

void core::CmdAttack::onQueue()
{
    // TODO: Reset frame to zero (since this is a new command)
}

bool core::CmdAttack::onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands)
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

std::string core::CmdAttack::toString() const
{
    return "attack";
}

core::Command* core::CmdAttack::clone()
{
    return ObjectPool<CmdAttack>::acquire(*this);
}

void core::CmdAttack::destroy()
{
    ObjectPool<CmdAttack>::release(this);
}

void core::CmdAttack::attack(int deltaTimeMs)
{
    const auto reloadTimeMs =
        1000.0f / (m_components->attack.attackRate * m_settings->getGameSpeed());
    timeSinceLastAttackMs += deltaTimeMs;

    if (timeSinceLastAttackMs >= reloadTimeMs)
    {
        timeSinceLastAttackMs = 0;

        auto [targetArmor, targetHealth] = m_stateMan->getComponents<CompArmor, CompHealth>(target);
        auto damage = getDamage(targetArmor);

        targetHealth.health -= damage;

        spdlog::debug("Attacked target {} for {} damage. Target health is now {}/{}", target,
                      damage, targetHealth.health, targetHealth.maxHealth.value());
    }
}

void core::CmdAttack::animate(int deltaTimeMs, int currentTick)
{
    m_components->action.action = UnitAction::ATTACK;
    auto& actionAnimation = m_components->animation.animations[m_components->action.action];

    auto ticksPerFrame = int(m_settings->getTicksPerSecond() /
                             (actionAnimation.value().speed * m_settings->getGameSpeed()));
    if (currentTick % ticksPerFrame == 0)
    {
        StateManager::markDirty(m_entityID);
        m_components->animation.frame++;
        m_components->animation.frame %= actionAnimation.value().frames; // Attacking is repeatable
    }
}

bool core::CmdAttack::isCloseEnough()
{
    debug_assert(target != entt::null, "Proposed entity to attack is null");

    return ProximityChecker::isInProximity(m_components->transform, target, m_stateMan.getRef());
}

bool core::CmdAttack::isComplete()
{
    // TODO - Perhaps there are other conditions for completion, such as target being destroyed
    // or no longer valid (e.g. out of LOS)
    auto& targetHealth = m_stateMan->getComponent<CompHealth>(target);
    return targetHealth.health <= 0;
}

void core::CmdAttack::moveCloser(std::list<Command*>& newCommands)
{
    debug_assert(target != entt::null, "Proposed entity to attack is null");

    const auto& targetPosition = m_stateMan->getComponent<CompTransform>(target).position;

    spdlog::debug("Target {} at {} (tile {}) is not close enough to build, moving...", target,
                  targetPosition.toString(), targetPosition.toTile().toString());
    auto moveCmd = ObjectPool<CmdMove>::acquire();
    moveCmd->targetEntity = target;
    moveCmd->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
    newCommands.push_back(moveCmd);
}

float core::CmdAttack::getDamage(const CompArmor& target) const
{
    float totalDamage = 0.0f;
    for (size_t i = 0; i < m_components->attack.attackPerClass.value().size(); ++i)
    {
        auto multipliedAttack = m_components->attack.attackPerClass[i] *
                                m_components->attack.attackMultiplierPerClass[i];
        auto damage = std::max(0.0f, multipliedAttack - target.armorPerClass[i]);
        totalDamage += damage;
    }
    totalDamage = std::max(1.0f, totalDamage * (1.0f - target.damageResistance.value()));
    return totalDamage;
}
