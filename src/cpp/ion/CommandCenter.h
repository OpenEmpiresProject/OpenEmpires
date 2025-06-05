#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include "EventHandler.h"

namespace ion
{
class Command;
class CommandCenter : public EventHandler
{
  public:
    CommandCenter();
    ~CommandCenter();

  private:
    void onInit(EventLoop* eventLoop) override;
    void onExit() override;
    void onEvent(const Event& e) override;
};
} // namespace ion

#endif