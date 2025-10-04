#ifndef GAMESTATE_H
#define GAMESTATE_H

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

    struct TileMapQueryResult
    {
        uint32_t entity = entt::null;
        MapLayerType layer = MapLayerType::MAX_LAYERS;
    };

    TileMapQueryResult whatIsAt(const Vec2& screenPos);

    bool canPlaceBuildingAt(const CompBuilding& building, const Feet& feet, bool& outOfMap);

  private:
    TileMap m_gameMap;
    entt::basic_registry<uint32_t> m_registry;
    Ref<PathFinderBase> m_pathFinder;
    std::vector<uint32_t> m_entitiesToDestroy;
};
} // namespace core

#endif