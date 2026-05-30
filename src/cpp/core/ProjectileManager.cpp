#include "ProjectileManager.h"

#include "components/CompArmor.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompHealth.h"
#include "components/CompProjectile.h"
#include "components/CompTransform.h"
#include "logging/Logger.h"
#include "utils/Maths.h"

using namespace core;

ProjectileManager::ProjectileManager()
{
    registerCallback(Event::Type::TICK, this, &ProjectileManager::onTick);
    registerCallback(Event::Type::PROJECTILE_CREATED, this, &ProjectileManager::onProjectileCreate);
}

bool ProjectileManager::onTick(const Event& e)
{
    auto& tickData = e.getData<TickData>();
    auto timeS = (double) tickData.deltaTimeMs / 1000.0;
    const auto collisionRadiusSq = PROJECTILE_COLLISION_RADIUS * PROJECTILE_COLLISION_RADIUS;
    auto& gameMap = m_stateMan->gameMap();

    for (auto it = m_projectilesTracking.begin(); it != m_projectilesTracking.end();)
    {
        auto projectile = *it;
        auto [projectileComp, transformComp, info, graphics] =
            m_stateMan->getComponents<CompProjectile, CompTransform, CompEntityInfo, CompGraphics>(
                projectile);

        transformComp.position += (projectileComp.direction *
                                   (projectileComp.speed * timeS * m_settings->getGameSpeed()));

        auto traveledDistanceSq =
            transformComp.position.distanceSquared(projectileComp.originPosition);

        if (graphics.debugOverlays.empty())
        {
            DebugOverlay anchor;
            anchor.type = DebugOverlay::Type::FILLED_CIRCLE;
            anchor.color = Color::PURPLE;
            anchor.circlePixelRadius = 10;
            graphics.debugOverlays.push_back(anchor);
        }
        graphics.debugOverlays[0].absolutePosition = transformComp.position;
        StateManager::markDirty(projectile);

        auto distanceToTarget =
            transformComp.position.distanceSquared(projectileComp.targetPosition);
        if (distanceToTarget < collisionRadiusSq)
        {
            // Hit or not, projectile is done
            if (projectileComp.damageMode == ProjectileDamageMode::ON_HIT)
            {
                auto hit = getHit(transformComp.position);
                if (hit != entt::null)
                {
                    auto [targetArmor, targetHealth] =
                        m_stateMan->getComponents<CompArmor, CompHealth>(hit);
                    auto damage = getDamage(projectileComp, targetArmor);

                    targetHealth.health -= damage;

                    spdlog::debug("HIT. Dealt {} damage, target health {}", damage,
                                  targetHealth.health);
                }
            }

            info.isDestroyed = true;
            m_stateMan->destroyEntity(projectile);
            it = m_projectilesTracking.erase(it);
            continue;
        }
        else if (traveledDistanceSq >= projectileComp.rangeSq)
        {
            // In case if the previous collision check didn't work, destroy the
            // projectile if it is out of range anyway as a fallback.
            info.isDestroyed = true;
            m_stateMan->destroyEntity(projectile);
            it = m_projectilesTracking.erase(it);
            continue;
        }
        ++it;
    }
    return false;
}

bool ProjectileManager::onProjectileCreate(const Event& e)
{
    auto& data = e.getData<ProjectileData>();

    spdlog::debug("Projectile {} received to track", data.projectileEntity);

    auto [projectile, transform] =
        m_stateMan->getComponents<CompProjectile, CompTransform>(data.projectileEntity);
    projectile = data.properties;
    projectile.originPosition = data.originPos;
    projectile.targetPosition = data.targetPos;
    projectile.direction = (data.targetPos - data.originPos).normalized();
    projectile.speed = data.speed;
    projectile.rangeSq = data.originPos.distanceSquared(data.targetPos);
    projectile.releaseHeight = data.releaseHeight;

    if (data.speed <= 0)
    {
        spdlog::warn("Projectile {} has invalid speed {}", data.projectileEntity, data.speed);
    }
    transform.position = data.originPos;
    m_projectilesTracking.push_back(data.projectileEntity);

    return false;
}

uint32_t ProjectileManager::getHit(const Feet& pos) const
{
    auto entity = m_stateMan->gameMap().getEntity(MapLayerType::STATIC, pos.toTile());
    if (entity != entt::null)
    {
        return entity;
    }

    auto units = m_stateMan->getUnitsAround(pos, std::nullopt, std::nullopt);
    for (auto unit : units)
    {
        auto& unitTransform = m_stateMan->getComponent<CompTransform>(unit);

        auto hit = maths::isOverlapping(unitTransform.position, unitTransform.collisionRadius, pos);
        if (hit)
        {
            return unit;
        }
    }
    return entt::null;
}

float ProjectileManager::getDamage(const ProjectileProperties& projectile,
                                   const CompArmor& target) const
{
    float totalDamage = 0.0f;
    for (size_t i = 0; i < projectile.attackPerClass.size(); ++i)
    {
        auto multipliedAttack =
            projectile.attackPerClass[i] * projectile.attackMultiplierPerClass[i];
        auto damage = std::max(0.0f, multipliedAttack - target.armorPerClass[i]);
        totalDamage += damage;
    }
    totalDamage = std::max(1.0f, totalDamage * (1.0f - target.damageResistance.value()));
    return totalDamage;
}