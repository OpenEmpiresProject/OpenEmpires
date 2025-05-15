#ifndef COMPDIRTY_H
#define COMPDIRTY_H

namespace aion
{
class CompDirty
{
  public:
    int64_t dirtyVersion = 0;          // Dirty counter for this entity
    inline static int64_t globalDirtyVersion = 0; // Global dirty counter to be used for all entities

    bool isDirty() const
    {
        return dirtyVersion == globalDirtyVersion;
    }

    void markDirty()
    {
        dirtyVersion = globalDirtyVersion;
    }
};

} // namespace aion

#endif