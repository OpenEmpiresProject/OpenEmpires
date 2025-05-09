#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "commands/Command.h"
#include "utils/ObjectPool.h"

namespace aion
{
class CmdAttack : public Command
{
  private:
    void onStart() override
    {
    }
    void onQueue(uint32_t entityID) override
    {
    }
    bool onExecute() override
    {
        return false;
    }
    std::string toString() const override
    {
        return "attack";
    };
    void destroy() override
    {
        ObjectPool<CmdAttack>::release(this);
    }
    bool onCreateSubCommands(std::list<Command*>& subCommands) override
    {
        return false;
    }
};

} // namespace aion

#endif