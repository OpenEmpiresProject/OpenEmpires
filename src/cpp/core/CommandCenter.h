#ifndef COMMANDCENTER_H
#define COMMANDCENTER_H

#include "EventHandler.h"

namespace core
{
class CommandCenter : public EventHandler
{
  public:
    CommandCenter();

    void onTick(const Event& e);
    void onCommandRequest(const Event& e);
};
} // namespace core

#endif