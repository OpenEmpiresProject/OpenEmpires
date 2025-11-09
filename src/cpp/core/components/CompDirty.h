#ifndef COMPDIRTY_H
#define COMPDIRTY_H

#include <cstdint>
#include <vector>
#include <set>

namespace core
{
class CompDirty
{
  public:
    // TODO: We might need to remove this class altogether. Had to add this dummy
    // otherwise entt can't handle empty classes as components.
    int dummy = 0;
    // Track all dirty entities - for a frame - by their id
    inline static std::set<uint32_t> g_dirtyEntities;

    void markDirty(uint32_t entityId)
    {
        g_dirtyEntities.insert(entityId);
    }
};

} // namespace core

#endif