#ifndef CMDBUILD_H
#define CMDBUILD_H

#include "Feet.h"
#include "GameState.h"
#include "Rect.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

namespace ion
{
class CmdBuild : public Command
{
  public:
    uint32_t target = entt::null;

  private:
    void onStart() override;
    void onQueue() override;
    bool onExecute(int deltaTimeMs, std::list<Command*>& subCommands) override;
    std::string toString() const override;
    void destroy() override;
    void animate(int deltaTimeMs);
    bool isCloseEnough();
    bool isComplete();
    void build(int deltaTimeMs);
    void moveCloser(std::list<Command*>& subCommands);
    bool overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect);

    float personalBuildingProgress = 0;
};
} // namespace ion

#endif