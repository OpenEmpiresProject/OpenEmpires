#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include "Logger.h"

#include <stop_token>

namespace aion
{
class SubSystem
{
  public:
    SubSystem(std::stop_token* stopToken) : stopToken_(stopToken) {};
    SubSystem(std::stop_source* stopSource) : stopSource_(stopSource) {};
    virtual ~SubSystem() = default;
    virtual void init() = 0;
    virtual void shutdown() = 0;

  protected:
    std::stop_token* stopToken_ = nullptr;
    std::stop_source* stopSource_ = nullptr;
};
} // namespace aion

#endif