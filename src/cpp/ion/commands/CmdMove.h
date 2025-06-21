#ifndef CMDMOVE_H
#define CMDMOVE_H

#include "Coordinates.h"
#include "Player.h"
#include "Vec2d.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompTransform.h"

#include <entt/entity/registry.hpp>
#include <list>

namespace ion
{
class CmdMove : public Command
{
  public:
    Vec2d goal; // In Feet

  private:
    std::list<Vec2d> path;
    uint32_t entity = entt::null;
    Ref<Coordinates> coordinates;
    Ref<Player> player;

    void onStart() override;
    void onQueue(uint32_t entityID) override;
    bool onExecute(uint32_t entityID, int deltaTimeMs) override;
    std::string toString() const override;
    void destroy() override;
    bool onCreateSubCommands(std::list<Command*>& subCommands) override;

    void animate(CompAction& action,
                 CompAnimation& animation,
                 CompDirty& dirty,
                 int deltaTimeMs,
                 uint32_t entityId);

    bool move(CompTransform& transform, int deltaTimeMs);
    std::list<Vec2d> findPath(const Vec2d& endPosInFeet, uint32_t entity);
    bool resolveCollision(const Vec2d& newPosFeet);
    Vec2d calculateNewPosition(CompTransform& transform, int timeMs);
    void setPosition(CompTransform& transform, const Vec2d& newPosFeet);
};
} // namespace ion

#endif