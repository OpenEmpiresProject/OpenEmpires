#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "CmdMove.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompArmor.h"
#include "components/CompAttack.h"
#include "components/CompHealth.h"
#include "components/CompTransform.h"
#include "utils/ObjectPool.h"

namespace core
{
class CmdAttack : public Command
{
  public:
    uint32_t target = entt::null;

  protected:
    float getDamage(const CompArmor& target) const;

  private:
    void onStart() override;
    void onQueue() override;
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override;
    std::string toString() const override;
    Command* clone() override;
    void destroy() override;

    void attack(int deltaTimeMs);
    void animate(int deltaTimeMs, int currentTick);
    bool isCloseEnough();
    bool isComplete();
    void moveCloser(std::list<Command*>& newCommands);
    static bool overlaps(const Feet& unitPos, float radiusSq, const Feet& targetPos);

  private:
    int timeSinceLastAttackMs = 0;
};

} // namespace core

#endif