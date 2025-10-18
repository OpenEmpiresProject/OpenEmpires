#ifndef CORE_CMDGARRISON_H
#define CORE_CMDGARRISON_H

#include "commands/Command.h"

namespace core
{
class Feet;
template <typename T> class Rect;

class CmdGarrison : public Command
{
  public:
    uint32_t target = entt::null;

  private:
    void onStart() override;
    void onQueue() override;
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override;
    std::string toString() const override;
    void destroy() override;
    bool isCloseEnough();
    bool isComplete();
    void garrison();
    void moveCloser(std::list<Command*>& subCommands);
    static bool overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect);
};
} // namespace core

#endif // CORE_CMDGARRISON_H
