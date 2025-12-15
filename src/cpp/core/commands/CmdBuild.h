#ifndef CMDBUILD_H
#define CMDBUILD_H

#include "commands/Command.h"

namespace core
{
class Feet;
template <typename T> class Rect;

class CmdBuild : public Command
{
  public:
    uint32_t target = entt::null;

  private:
    float m_constructionContribution = 0;

  private:
    void onStart() override;
    void onQueue() override;
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override;
    std::string toString() const override;
    void destroy() override;
    void animate(int deltaTimeMs, int currentTick);
    bool isCloseEnough();
    bool isComplete();
    void build(int deltaTimeMs);
    void moveCloser(std::list<Command*>& newCommands);
    void lookForAnotherSiteToBuild(std::list<Command*>& newCommands);
    static bool overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect);
};
} // namespace core

#endif