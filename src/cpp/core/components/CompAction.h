#ifndef COMPACTION_H
#define COMPACTION_H

namespace core
{
class CompAction
{
  public:
    int action = 0;

    CompAction(int action) : action(action)
    {
    }
};
} // namespace core

#endif