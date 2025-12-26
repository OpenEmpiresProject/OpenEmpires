#ifndef COMPACTION_H
#define COMPACTION_H

namespace core
{
class CompAction
{
  public:
    int action = 0;

    static constexpr auto properties()
    {
        return std::tuple{/* No properties for now */};
    }
};
} // namespace core

#endif