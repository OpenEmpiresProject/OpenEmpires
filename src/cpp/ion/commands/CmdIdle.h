#ifndef CMDIDLEH
#define CMDIDLEH

#include "GameState.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

namespace ion
{
class CmdIdle : public Command
{
  public:
    CmdIdle(uint32_t entityId)
    {
        m_entityID = entityId;
    }

  private:
    void onStart() override
    {
    }

    void onQueue() override
    {
    }

    bool onExecute(int deltaTimeMs, std::list<Command*>& subCommands) override
    {
        auto [action, animation, dirty] =
            m_gameState->getComponents<CompAction, CompAnimation, CompDirty>(m_entityID);
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

    void animate(CompAction& action, CompAnimation& animation, CompDirty& dirty)
    {
        action.action = UnitAction::IDLE;
        auto& actionAnimation = animation.animations[action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty(m_entityID);
            animation.frame++;
            animation.frame %= actionAnimation.frames; // Idle is always repeatable
        }
    }
};
} // namespace ion

#endif