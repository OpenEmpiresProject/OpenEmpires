#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "Coordinates.h"
#include "PassabilityMap.h"
#include "TileMap.h"

#include <entt/entity/registry.hpp>
#include <vector>

namespace core
{
class PathFinderBase;
class CompBuilding;

class StateManager
{
  public:
    StateManager();
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;

    void init();

    uint32_t createEntity();

    // Safe to call this at any time without any synchronization, but not thread safe
    void destroyEntity(uint32_t entity);
    bool isEntityValid(uint32_t entity) const;

    // This should be called in a synchronized manner with all stakeholders of components
    void destroyAllPendingEntities();

    template <typename T, typename... Args>
    decltype(auto) addComponent(uint32_t entity, Args&&... args)
    {
        return m_registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T> decltype(auto) addComponent(uint32_t entity, const T& t)
    {
        return m_registry.emplace_or_replace<T>(entity, t);
    }

    template <typename T> bool hasComponent(uint32_t entity) const
    {
        return m_registry.all_of<T>(entity);
    }

    template <typename T> T* tryGetComponent(uint32_t entity)
    {
        return m_registry.try_get<T>(entity);
    }

    template <typename T> T& getComponent(uint32_t entity)
    {
        return m_registry.get<T>(entity);
    }

    template <typename... T> decltype(auto) getComponents(uint32_t entity)
    {
        return m_registry.get<T...>(entity);
    }

    // write a function get all entities with all given components
    template <typename... T> auto getEntities()
    {
        return m_registry.view<T...>();
    }

    void clearAll();

    Ref<PathFinderBase> getPathFinder() const
    {
        return m_pathFinder;
    }

    TileMap& gameMap()
    {
        return m_gameMap;
    }

    PassabilityMap& getPassabilityMap()
    {
        return m_passabilityMap;
    }

    struct TileMapQueryResult
    {
        uint32_t entity = entt::null;
        MapLayerType layer = MapLayerType::MAX_LAYERS;
    };

    TileMapQueryResult whatIsAt(const Vec2& screenPos);

    bool canPlaceBuildingAt(const CompBuilding& building, bool& outOfMap);

    auto& getRegistry()
    {
        return m_registry;
    }

    static std::set<uint32_t>& getDirtyEntities();
    static void markDirty(uint32_t entityId);
    static void clearDirtyEntities();

  private:
    TileMap m_gameMap;
    PassabilityMap m_passabilityMap;
    entt::basic_registry<uint32_t> m_registry;
    Ref<PathFinderBase> m_pathFinder;
    std::vector<uint32_t> m_entitiesToDestroy;
    Ref<Coordinates> m_coordinates;

    inline static std::set<uint32_t> g_dirtyEntities;
};
} // namespace core

#endif