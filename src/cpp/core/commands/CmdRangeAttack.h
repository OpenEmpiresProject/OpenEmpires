#ifndef CORE_CMDRANGEATTACK_H
#define CORE_CMDRANGEATTACK_H

#include "Command.h"
#include "EntityFactory.h"

namespace core
{
class CmdRangeAttack : public Command
{
  public:
    uint32_t target = entt::null;

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

    int timeSinceLastAnimationEndMs = 0;
    bool createProjectile = false;

    LazyServiceRef<EntityFactory> m_entityFactory;
};
} // namespace core

#endif // CORE_CMDRANGEATTACK_H
