#ifndef CMDWALK_H
#define CMDWALK_H

#include "Vec2d.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompTransform.h"
#include "utils/ObjectPool.h"

#include <list>

namespace aion
{
class CmdWalk : public Command
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

        animate(action, animation, dirty, deltaTimeMs);
        return walk(transform, deltaTimeMs);
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

    void animate(CompAction& action, CompAnimation& animation, CompDirty& dirty, int deltaTimeMs)
    {
        action.action = 1; // TODO: Not good
        auto& actionAnimation = animation.animations[action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty();
            animation.frame++;
            animation.frame %= actionAnimation.frames;
        }
    }

    bool walk(CompTransform& transform, int deltaTimeMs)
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
                transform.walk(deltaTimeMs); // TODO: calcu time accurately
            }
        }

        return path.empty();
    }
};
} // namespace aion

#endif