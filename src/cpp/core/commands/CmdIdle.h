#ifndef CMDIDLEH
#define CMDIDLEH

#include "StateManager.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "logging/Logger.h"
#include "utils/ObjectPool.h"

namespace core
{
class CmdIdle : public Command
{
  public:
    CmdIdle() = default;
    CmdIdle(uint32_t entityId)
    {
        m_entityID = entityId;
    }

    CmdIdle(const CmdIdle& other)
    {
        *this = other;
    }

  private:
    void onStart() override
    {
        // Calling setEntityID explicitly to populate m_components since the CmdIdle was
        // constructed before having ability to set it up properly (since idle is the default
        // command).
        setEntityID(m_entityID);
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
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override
    {
        animate(currentTick);
        return false; // Idling never completes
    }

    std::string toString() const override
    {
        return "idle";
    }

    Command* clone() override
    {
        return ObjectPool<CmdIdle>::acquire(*this);
    }

    void destroy() override
    {
        ObjectPool<CmdIdle>::release(this);
    }

    void animate(int currentTick)
    {
        m_components->action.action = UnitAction::IDLE;
        const auto& actionAnimation = m_components->animation.animations[UnitAction::IDLE];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (currentTick % ticksPerFrame == 0)
        {
            StateManager::markDirty(m_entityID);
            m_components->animation.frame++;
            m_components->animation.frame %= actionAnimation.frames; // Idle is always repeatable
        }
    }
};
} // namespace core

#endif