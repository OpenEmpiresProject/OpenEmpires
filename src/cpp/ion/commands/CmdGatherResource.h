#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "commands/Command.h"
#include "utils/ObjectPool.h"
#include "Coordinates.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "Vec2d.h"
#include "commands/CmdMove.h"
#include "components/CompResource.h"
#include "components/CompTransform.h"
#include "components/CompSelectible.h"
#include "components/CompEntityInfo.h"

namespace ion
{
class CmdGatherResource : public Command
{
public:
    uint32_t target = entt::null;

private:
    uint32_t entity = entt::null;
    Vec2d targetPosition; // TODO: Use when target is absent by the time this command execute
    const int choppingSpeed = 10; // 10 wood per second
    float collectedWood = 0;


    void onStart() override
    {
        spdlog::debug("Start gathering resource...");
    }

    void onQueue(uint32_t entityID) override
    {
        // TODO: Reset frame to zero (since this is a new command)
        entity = entityID;
        auto& transformTarget = GameState::getInstance().getComponent<CompTransform>(target);
        targetPosition = transformTarget.position;
    }

    bool onExecute(uint32_t entityID, int deltaTimeMs) override
    {
        if (isCloseEnough())
        {
            auto [transform, action, animation, dirty] =
            GameState::getInstance()
                .getComponents<CompTransform, CompAction, CompAnimation, CompDirty>(entityID);

            animate(action, animation, dirty, deltaTimeMs, entityID);
            return gather(transform, deltaTimeMs);
        }
        return false;
    }

    std::string toString() const override
    {
        return "gather-resource";
    }

    void destroy() override
    {
        ObjectPool<CmdGatherResource>::release(this);
    }

    bool onCreateSubCommands(std::list<Command*>& subCommands) override
    {
        if (!isCloseEnough())
        {
            spdlog::debug("Target is not close enough, moving...");
            auto move = ObjectPool<CmdMove>::acquire();
            move->goal = targetPosition;
            move->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
            subCommands.push_back(move);
            return true;
        }
        return false;
    }

    bool isCloseEnough()
    {
        auto& transformTarget = GameState::getInstance().getComponent<CompTransform>(target);
        auto& transformMy = GameState::getInstance().getComponent<CompTransform>(entity);
        return transformMy.position.distanceSquared(transformTarget.position) <
               transformTarget.goalRadiusSquared;
    }

    void animate(CompAction& action,
                 CompAnimation& animation,
                 CompDirty& dirty,
                 int deltaTimeMs,
                 uint32_t entityId)
    {
        action.action = 2; // TODO: Not good
        auto& actionAnimation = animation.animations[action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty(entityId);
            animation.frame++;
            animation.frame %= actionAnimation.frames;
        }
    }

    bool gather(CompTransform& transform, int deltaTimeMs)
    {
        collectedWood += float(choppingSpeed) * deltaTimeMs / 1000.0f;
        int rounded = collectedWood;;
        collectedWood -= rounded;

        if (rounded != 0)
        {
            cutTree(rounded);
        }
        return false;
    }

    void cutTree(uint16_t delta)
    {
        auto tree = target;

        if (GameState::getInstance().hasComponent<CompResource>(tree))
        {
            auto [resource, dirty, info, select] =
                GameState::getInstance()
                    .getComponents<CompResource, CompDirty, CompEntityInfo, CompSelectible>(tree);
            resource.resource.amount =
                resource.resource.amount < delta ? 0 : resource.resource.amount - delta;
            if (resource.resource.amount == 0)
            {
                info.isDestroyed = true;
            }
            else
            {
                info.entitySubType = 1;
                info.variation = 0; //  regardless of the tree type, this is the chopped version
                // TODO: Might not be the most optimal way to bring down the bounding box a chopped
                // tree
                auto tw = Constants::TILE_PIXEL_WIDTH;
                auto th = Constants::TILE_PIXEL_HEIGHT;
                select.boundingBoxes[static_cast<int>(Direction::NONE)] =
                    Rect<int>(tw / 2, th / 2, tw, th);
            }
            dirty.markDirty(tree);
            spdlog::info("Tree has {} resources", resource.resource.amount);
        }
        else
        {
            spdlog::error("Target entity {} is not a resouce", target);
        }
    }
};

} // namespace ion

#endif