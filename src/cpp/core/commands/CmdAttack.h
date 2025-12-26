#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "commands/Command.h"
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
};

} // namespace core

#endif