#ifndef CORE_CMDATTACK_H
#define CORE_CMDATTACK_H
#include "Command.h"

namespace core
{
class CmdAttack : public Command
{
  public:
    uint32_t target = entt::null;

    void onStart() override;
    void onQueue() override;
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override;
    std::string toString() const override;
    Command* clone() override;
    void destroy() override;

  private:
    // This is the actual command while CmdAttack act as the command delegator.
    Command* m_attackCommand = nullptr;
};
} // namespace core

#endif // CORE_CMDATTACK_H
