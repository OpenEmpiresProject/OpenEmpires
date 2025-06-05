#ifndef COMPUNIT_H
#define COMPUNIT_H

#include "commands/Command.h"

#include <queue>
#include <vector>

namespace ion
{
using CommandQueueType = std::priority_queue<Command*, std::vector<Command*>, CompareTaskPtr>;

class CompUnit
{
  public:
    CommandQueueType commandQueue;
};

} // namespace ion

#endif