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

    bool onExecute(int deltaTimeMs, std::list<Command*>& subCommands) override
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
};

} // namespace core

#endif