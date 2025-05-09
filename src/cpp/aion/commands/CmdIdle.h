#ifndef CMDIDLEH
#define CMDIDLEH

#include "commands/Command.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

namespace aion
{
class CmdIdle : public Command
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
        spdlog::debug("Idling...");
        return false;
    }
    std::string toString() const override
    {
        return "idle";
    };
    void destroy() override
    {
        ObjectPool<CmdIdle>::release(this);
    }
    bool onCreateSubCommands(std::list<Command*>& subCommands) override
    {
        return false;
    }
};
} // namespace aion

#endif