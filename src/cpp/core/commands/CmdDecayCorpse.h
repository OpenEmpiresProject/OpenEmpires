#ifndef CMD_DECAY_CORPSE_H
#define CMD_DECAY_CORPSE_H

#include "Event.h"
#include "EventPublisher.h"
#include "StateManager.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "logging/Logger.h"
#include "utils/ObjectPool.h"

namespace core
{
class CmdDecayCorpse : public Command
{
  private:
    void onStart() override
    {
        m_components->animation.frame = 0;
    }

    void onQueue() override
    {
        spdlog::debug("Command received to decay the corpse of the died entity {}", m_entityID);
    }

    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override
    {
        return animate(currentTick);
    }

    std::string toString() const override
    {
        return "decay_corpse";
    }

    Command* clone() override
    {
        return ObjectPool<CmdDecayCorpse>::acquire(*this);
    }

    void destroy() override
    {
        ObjectPool<CmdDecayCorpse>::release(this);
    }

    bool animate(int currentTick)
    {
        m_components->action.action = UnitAction::DECAY_CORPSE;
        const auto& actionAnimation = m_components->animation.animations[UnitAction::DECAY_CORPSE];
        bool completed = false;

        // TODO: speed should be float to accept very low speed animations like corpse decay
        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (currentTick % (int)ticksPerFrame == 0)
        {
            StateManager::markDirty(m_entityID);
            m_components->animation.frame++;

            spdlog::debug("Decay corpse animation frame {}", m_components->animation.frame);

            completed = m_components->animation.frame >= actionAnimation.frames;
        }

        if (completed)
        {
            m_components->animation.frame--;
            Event event(Event::Type::ENTITY_DELETE, EntityDeleteData{m_entityID});
            publishEvent(event);
        }
        return completed;
    }
};
} // namespace core

#endif