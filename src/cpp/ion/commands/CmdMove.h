#ifndef CMDMOVE_H
#define CMDMOVE_H

#include "Vec2d.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompTransform.h"
#include "utils/ObjectPool.h"

#include <list>

namespace ion
{
class CmdMove : public Command
{
  public:
    // TODO: this is temporary. need flow-field and goal position only.
    std::list<Vec2d> path;

  private:
    void onStart() override
    {
    }

    void onQueue(uint32_t entityID) override
    {
    }

    bool onExecute(uint32_t entityID, int deltaTimeMs) override
    {
        auto [transform, action, animation, dirty] =
            GameState::getInstance()
                .getComponents<CompTransform, CompAction, CompAnimation, CompDirty>(entityID);

        animate(action, animation, dirty, deltaTimeMs, entityID);
        return move(transform, deltaTimeMs);
    }

    std::string toString() const override
    {
        return "move";
    }

    void destroy() override
    {
        ObjectPool<CmdMove>::release(this);
    }

    bool onCreateSubCommands(std::list<Command*>& subCommands) override
    {
        return false;
    }

    void animate(CompAction& action,
                 CompAnimation& animation,
                 CompDirty& dirty,
                 int deltaTimeMs,
                 uint32_t entityId)
    {
        action.action = 1; // TODO: Not good
        auto& actionAnimation = animation.animations[action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty(entityId);
            animation.frame++;
            animation.frame %= actionAnimation.frames;
        }
    }

    bool move(CompTransform& transform, int deltaTimeMs)
    {
        if (!path.empty())
        {
            auto& nextPos = path.front();
            if (transform.position.distanceSquared(nextPos) < transform.goalRadiusSquared)
            {
                path.pop_front();
            }
            else
            {
                transform.face(nextPos);
                transform.move(deltaTimeMs);
            }
        }
        return path.empty();
    }
};
} // namespace ion

#endif