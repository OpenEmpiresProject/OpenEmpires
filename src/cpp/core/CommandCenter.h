#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include "EventHandler.h"

namespace core
{
class CommandCenter : public EventHandler
{
  private:
    void onInit(EventLoop* eventLoop) override;
    void onExit() override;
    void onEvent(const Event& e) override;
};
} // namespace core

#endif