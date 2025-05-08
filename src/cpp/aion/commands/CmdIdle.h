#ifndef CMDIDLEH
#define CMDIDLEH

#include "Logger.h"
#include "ObjectPool.h"
#include "commands/Command.h"

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
        utils::ObjectPool<CmdIdle>::release(this);
    }
    bool onCreateSubCommands(std::list<Command*>& subCommands) override
    {
        return false;
    }
};
} // namespace aion

#endif