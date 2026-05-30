#ifndef CORE_COMPRANGEATTACK_H
#define CORE_COMPRANGEATTACK_H
#include "Property.h"
#include "utils/Types.h"

namespace core
{
class CompRangeAttack
{
  public:
    Property<ProjectileProperties> primaryProjectile;
    Property<std::vector<ProjectileProperties>> secondaryProjectiles;
    Property<ProjectileDamageMode> damageMode;
    Property<uint32_t> projectileEntityType;
    Property<int> attackRange;
    Property<int> projectileSpeed;
    Property<int> projectileReleaseFrame;
    Property<int> projectileReleaseHeight;
};
} // namespace core

#endif // CORE_COMPRANGEATTACK_H
