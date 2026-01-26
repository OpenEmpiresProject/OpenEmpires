#ifndef COMPUNIT_H
#define COMPUNIT_H

#include "Property.h"
#include "commands/Command.h"

#include <queue>
#include <vector>

namespace core
{
using CommandQueueType = std::priority_queue<Command*, std::vector<Command*>, CompareTaskPtr>;

class CompUnit
{
  public:
    Property<uint32_t> housingNeed;
    Property<UnitType> type;

  public:
    Property<Ref<Command>> defaultCommand;

    CommandQueueType commandQueue;
    bool isGarrisoned = false;

    void onCreate(uint32_t entity)
    {
        while (not commandQueue.empty())
        {
            commandQueue.pop();
        }
        auto cloned = defaultCommand.value()->clone();
        cloned->setEntityID(entity);
        commandQueue.push(cloned);
    }
};

} // namespace core

#endif