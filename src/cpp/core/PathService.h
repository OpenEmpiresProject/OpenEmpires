#ifndef CORE_PATHSERVICE_H
#define CORE_PATHSERVICE_H

#include "Feet.h"
#include "Path.h"
#include "StateManager.h"

namespace core
{
class PathFinderBase;
class Player;
class PathService
{
  public:
    PathService();
    ~PathService();

    Path findPath(const Feet& from, const Feet& to, Ref<Player> player);
    void refinePath(Path& path, Ref<Player> player) const;
    bool canTraverseDirectly(const Feet& from, const Feet& to, Ref<Player> player) const;

  private:
    Ref<PathFinderBase> m_pathFinder;
    LazyServiceRef<StateManager> m_stateMan;
    const int m_maxDirectPathDistanctInFeetSquared;
};
} // namespace core

#endif // CORE_PATHSERVICE_H
