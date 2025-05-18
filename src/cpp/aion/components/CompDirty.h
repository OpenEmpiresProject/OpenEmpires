#ifndef COMPDIRTY_H
#define COMPDIRTY_H

namespace aion
{
class CompDirty
{
  public:
    int64_t dirtyVersion = 0;          // Dirty counter for this entity
    inline static int64_t globalDirtyVersion = 0; // Global dirty counter to be used for all entities
    inline static std::vector<uint32_t> g_dirtyEntities;

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

} // namespace aion

#endif