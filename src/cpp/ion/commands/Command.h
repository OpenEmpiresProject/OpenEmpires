#ifndef COMMAND_H
#define COMMAND_H

#include "GameSettings.h"
#include "ServiceRegistry.h"
#include "utils/Types.h"

#include <entt/entity/registry.hpp>
#include <list>

namespace ion
{
class Command
{
  public:
    static inline constexpr int DEFAULT_PRIORITY = 0;
    static inline constexpr int CHILD_PRIORITY_OFFSET = 1000;

    Command()
    {
        m_settings = ServiceRegistry::getInstance().getService<GameSettings>();
    }

    virtual ~Command() = default;

    virtual void onStart() = 0;
    virtual void onQueue() = 0;
    virtual bool onExecute(int deltaTimeMs, std::list<Command*>& subCommands) = 0;
    virtual std::string toString() const = 0;
    virtual void destroy() = 0;

    void setPriority(int m_priority)
    {
        this->m_priority = m_priority;
    }

    int getPriority() const
    {
        return m_priority;
    }

    static void incrementTotalTicks()
    {
        s_totalTicks++;
    }

    void setEntityID(uint32_t entityID)
    {
        m_entityID = entityID;
    }

    inline bool isExecutedAtLeastOnce() const
    {
        return m_executedAtLeastOnce;
    }

    inline void setExecutedAtLeastOnce(bool executed)
    {
        m_executedAtLeastOnce = executed;
    }

  protected:
    inline static int s_totalTicks = 0;

    std::shared_ptr<GameSettings> m_settings;
    int m_priority = 0;
    uint32_t m_entityID = entt::null;

    bool m_executedAtLeastOnce = false;
};

// Comparator to use Commands in priority_queue
struct CompareTaskPtr
{
    bool operator()(const Command* a, const Command* b) const
    {
        return a->getPriority() < b->getPriority(); // max-heap: higher m_priority = top
    }
};

} // namespace ion

#endif