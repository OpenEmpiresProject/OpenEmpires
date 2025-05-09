#ifndef CMDWALK_H
#define CMDWALK_H

#include "Vec2d.h"
#include "commands/Command.h"
#include "utils/ObjectPool.h"

#include <vector>

namespace aion
{
class CmdWalk : public Command
{
  public:
    // TODO: this is temporary. need flow-field and goal position only.
    std::vector<Vec2d> path;

  private:
    int tempTickCount = 0;
    void onStart() override
    {
    }
    void onQueue(uint32_t entityID) override
    {
    }
    bool onExecute() override
    {
        tempTickCount++;
        spdlog::debug("Walking...");
        if (tempTickCount % 10 == 0)
        {
            path.erase(path.begin());
        }
        return path.empty();
    }
    std::string toString() const override
    {
        return "walk";
    };
    void destroy() override
    {
        ObjectPool<CmdWalk>::release(this);
    }
    bool onCreateSubCommands(std::list<Command*>& subCommands) override
    {
        return false;
    }
};
} // namespace aion

#endif