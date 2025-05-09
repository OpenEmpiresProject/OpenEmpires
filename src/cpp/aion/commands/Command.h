#ifndef COMMAND_H
#define COMMAND_H

#include "utils/Types.h"

#include <list>

namespace aion
{
class Command
{
  public:
    virtual ~Command()
    {
    }
    virtual void onStart() = 0;
    virtual void onQueue(uint32_t entityID) = 0;
    virtual bool onExecute() = 0;
    virtual std::string toString() const = 0;
    virtual void destroy() = 0;
    virtual bool onCreateSubCommands(std::list<Command*>& subCommands) = 0;

    void setPriority(int priority)
    {
        this->priority = priority;
    }

    int getPriority() const
    {
        return priority;
    }

  private:
    int priority = 0;
};

struct CompareTaskPtr
{
    bool operator()(const Command* a, const Command* b) const
    {
        return a->getPriority() < b->getPriority(); // max-heap: higher priority = top
    }
};

} // namespace aion

#endif