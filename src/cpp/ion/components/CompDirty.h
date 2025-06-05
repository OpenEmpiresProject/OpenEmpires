#ifndef COMPDIRTY_H
#define COMPDIRTY_H

namespace ion
{
class CompDirty
{
  public:
    // Dirty counter for this entity.
    int64_t dirtyVersion = 0;
    // Global dirty counter to be used for all entities. This aligns with frame number;
    inline static int64_t globalDirtyVersion = 0;
    // Track all dirty entities - for a frame - by their id
    inline static std::vector<uint32_t> g_dirtyEntities;

    // A faster version to check the dirtyness without iterating through g_dirtyEntities
    bool isDirty() const
    {
        return dirtyVersion == globalDirtyVersion;
    }

    void markDirty(uint32_t entityId)
    {
        dirtyVersion = globalDirtyVersion;
        g_dirtyEntities.push_back(entityId);
    }
};

} // namespace ion

#endif