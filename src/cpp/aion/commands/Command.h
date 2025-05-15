#ifndef COMMAND_H
#define COMMAND_H

#include "GameSettings.h"
#include "ServiceRegistry.h"
#include "utils/Types.h"

#include <list>

namespace aion
{
class Command
{
  public:
    Command()
    {
        m_settings = ServiceRegistry::getInstance().getService<GameSettings>();
    }

    virtual ~Command() = default;

    virtual void onStart() = 0;
    virtual void onQueue(uint32_t entityID) = 0;
    virtual bool onExecute(uint32_t entityID, int deltaTimeMs) = 0;
    virtual std::string toString() const = 0;
    virtual void destroy() = 0;
    virtual bool onCreateSubCommands(std::list<Command*>& subCommands) = 0;

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

  protected:
    std::shared_ptr<GameSettings> m_settings;
    int m_priority = 0;
    inline static int s_totalTicks = 0;
};

// Comparator to use Commands in priority_queue
struct CompareTaskPtr
{
    bool operator()(const Command* a, const Command* b) const
    {
        return a->getPriority() < b->getPriority(); // max-heap: higher m_priority = top
    }
};

} // namespace aion

#endif