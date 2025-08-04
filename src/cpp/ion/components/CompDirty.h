#ifndef COMPDIRTY_H
#define COMPDIRTY_H

#include <cstdint>
#include <vector>

namespace ion
{
class CompDirty
{
  public:
    // TODO: We might need to remove this class altogether. Had to add this dummy
    // otherwise entt can't handle empty classes as components.
    int dummy = 0;
    // Track all dirty entities - for a frame - by their id
    inline static std::vector<uint32_t> g_dirtyEntities;

    void markDirty(uint32_t entityId)
    {
        g_dirtyEntities.push_back(entityId);
    }
};

} // namespace ion

#endif