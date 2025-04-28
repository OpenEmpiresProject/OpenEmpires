#ifndef DIRTYCOMPONENT_H
#define DIRTYCOMPONENT_H

#include "Component.h"

namespace aion
{
class DirtyComponent : public aion::Component<DirtyComponent>
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

inline int64_t DirtyComponent::globalDirtyVersion =
    0; // Global dirty counter to be used for all entities

} // namespace aion

#endif