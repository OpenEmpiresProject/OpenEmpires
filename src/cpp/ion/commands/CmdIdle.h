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

    /**
     * Executes the idle command for the specified entity.
     *
     * This method performs the animation update. The idle command never completes, so it always
     * returns false. As in it would be the fallback action. Any other action must have a higher
     * priority to get executed.
     *
     * @param deltaTimeMs The elapsed time in milliseconds since the last execution.
     * @param subCommands A list to which any sub-commands can be added (unused in idle).
     * @return Always returns false, indicating the idle command never completes.
     */
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

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.value().speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty(m_entityID);
            animation.frame++;
            animation.frame %= actionAnimation.value().frames; // Idle is always repeatable
        }
    }
};
} // namespace ion

#endif