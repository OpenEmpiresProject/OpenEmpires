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
        // TODO: Reset frame to zero (since this is a new command)
    }

    bool onExecute(uint32_t entityID, int deltaTimeMs) override
    {
        return false;
    }

    std::string toString() const override
    {
        return "attack";
    }

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