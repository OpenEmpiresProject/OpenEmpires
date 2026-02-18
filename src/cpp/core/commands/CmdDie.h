#ifndef CMDDIE_H
#define CMDDIE_H

#include "CmdDecayCorpse.h"
#include "Event.h"
#include "EventPublisher.h"
#include "StateManager.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompUnit.h"
#include "logging/Logger.h"
#include "utils/ObjectPool.h"

namespace core
{
class CmdDie : public Command
{
  private:
    void onStart() override
    {
        m_components->animation.frame = 0;
    }

    void onQueue() override
    {
        spdlog::debug("Request received to die for entity {}", m_entityID);
    }

    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override
    {
        bool completed = animate(currentTick);
        if (completed)
        {
            convertToCorpse(subCommands);
        }
        return completed;
    }

    std::string toString() const override
    {
        return "die";
    }

    Command* clone() override
    {
        return ObjectPool<CmdDie>::acquire(*this);
    }

    void destroy() override
    {
        ObjectPool<CmdDie>::release(this);
    }

    bool animate(int currentTick)
    {
        m_components->action.action = UnitAction::DIE;
        const auto& actionAnimation = m_components->animation.animations[UnitAction::DIE];
        bool animationCompleted = false;

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (currentTick % (int)ticksPerFrame == 0)
        {
            StateManager::markDirty(m_entityID);
            m_components->animation.frame++;

            spdlog::debug("Death animation frame {}", m_components->animation.frame);

            animationCompleted = m_components->animation.frame >= actionAnimation.frames;
        }

        if (animationCompleted)
        {
            m_components->animation.frame--;
            //             Event event(Event::Type::ENTITY_DELETE, EntityDeleteData{m_entityID});
            //             publishEvent(event);
        }
        return animationCompleted;
    }

    void convertToCorpse(std::list<Command*>& subCommands)
    {
        spdlog::debug("Target {} death is completed, converting to a corpse", m_entityID);

        while (not m_components->unit.commandQueue.empty())
        {
            m_components->unit.commandQueue.pop();
        }
        auto moveCmd = ObjectPool<CmdDecayCorpse>::acquire();
        subCommands.push_back(moveCmd);
    }
};
} // namespace core

#endif