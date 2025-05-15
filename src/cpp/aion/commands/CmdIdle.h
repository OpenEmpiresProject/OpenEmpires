#ifndef CMDIDLEH
#define CMDIDLEH

#include "GameState.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
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

    bool onExecute(uint32_t entityID, int deltaTimeMs) override
    {
        auto [action, animation, dirty] =
            GameState::getInstance().getComponents<CompAction, CompAnimation, CompDirty>(entityID);
        animate(action, animation, dirty);
        return false; // Idling never completes
    }

    std::string toString() const override
    {
        return "idle";
    }

    void destroy() override
    {
        ObjectPool<CmdIdle>::release(this);
    }

    bool onCreateSubCommands(std::list<Command*>& subCommands) override
    {
        return false;
    }

    void animate(CompAction& action, CompAnimation& animation, CompDirty& dirty)
    {
        action.action = 0; // TODO: Not good
        auto& actionAnimation = animation.animations[action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty();
            animation.frame++;
            animation.frame %= actionAnimation.frames; // Idle is always repeatable
        }
    }
};
} // namespace aion

#endif