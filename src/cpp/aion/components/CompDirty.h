#ifndef COMPDIRTY_H
#define COMPDIRTY_H

namespace aion
{
class CompDirty
{
  public:
    int64_t dirtyVersion = 0;          // Dirty counter for this entity
    static int64_t globalDirtyVersion; // Global dirty counter to be used for all entities

    bool isDirty() const
    {
        return dirtyVersion == globalDirtyVersion;
    }

    void markDirty()
    {
        dirtyVersion = globalDirtyVersion;
    }
};

inline int64_t CompDirty::globalDirtyVersion = 0;

} // namespace aion

#endif