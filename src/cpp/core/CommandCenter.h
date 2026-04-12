#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include "EventHandler.h"

namespace core
{
class CommandCenter : public EventHandler
{
  public:
    CommandCenter();

    bool onTick(const Event& e);
    bool onCommandRequest(const Event& e);
};
} // namespace core

#endif