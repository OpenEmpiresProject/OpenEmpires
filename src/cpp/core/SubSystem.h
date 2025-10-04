#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

#include <stop_token>

namespace core
{
class SubSystem
{
  public:
    SubSystem(std::stop_token* stopToken) : m_stopToken(stopToken) {};
    SubSystem(std::stop_source* stopSource) : m_stopSource(stopSource) {};
    virtual ~SubSystem() = default;
    virtual void init() = 0;
    virtual void shutdown() = 0;

  protected:
    std::stop_token* m_stopToken = nullptr;
    std::stop_source* m_stopSource = nullptr;
};
} // namespace core

#endif