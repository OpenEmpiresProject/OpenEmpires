#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "commands/Command.h"
#include "components/CompArmor.h"
#include "components/CompAttack.h"
#include "utils/ObjectPool.h"

namespace core
{
class CmdAttack : public Command
{
  private:
    void onStart() override
    {
    }

    void onQueue() override
    {
        // TODO: Reset frame to zero (since this is a new command)
    }

    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override
    {
        return false;
    }

    std::string toString() const override
    {
        return "attack";
    }

    Command* clone() override
    {
        return ObjectPool<CmdAttack>::acquire(*this);
    }

    void destroy() override
    {
        ObjectPool<CmdAttack>::release(this);
    }

    float getDamage(const CompArmor& target) const
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
};

} // namespace core

#endif