#ifndef CORE_PROJECTILEMANAGER_H
#define CORE_PROJECTILEMANAGER_H
#include "EventHandler.h"
#include "StateManager.h"
#include "components/CompArmor.h"

namespace core
{
class StateManager;

class ProjectileManager : public EventHandler
{
  public:
    ProjectileManager();

  private:
    bool onTick(const Event& e);
    bool onProjectileCreate(const Event& e);
    uint32_t getHit(const Feet& pos) const;
    float getDamage(const ProjectileProperties& projectile, const CompArmor& target) const;

  private:
    std::list<uint32_t> m_projectilesTracking;
    LazyServiceRef<StateManager> m_stateMan;
    LazyServiceRef<Settings> m_settings;

    const int PROJECTILE_COLLISION_RADIUS = 10;
};
} // namespace core

#endif // CORE_PROJECTILEMANAGER_H
