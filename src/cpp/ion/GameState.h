#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "PathFinderBase.h"
#include "ServiceRegistry.h"
#include "SubSystem.h"
#include "TileMap.h"

#include <entt/entity/registry.hpp>
#include <vector>

namespace ion
{
class GameState
{
  public:
    GameState();
    ~GameState();
    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;

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

    TileMap gameMap;

    PathFinderBase* getPathFinder() const
    {
        return m_pathFinder;
    }

  private:
    entt::basic_registry<uint32_t> m_registry;
    PathFinderBase* m_pathFinder = nullptr;
    std::vector<uint32_t> m_entitiesToDestroy;
};

class Entity
{
  public:
    template <typename T> static decltype(auto) addComponent(uint32_t entity, const T& t)
    {
        return ServiceRegistry::getInstance().getService<GameState>()->addComponent<T>(entity, t);
    }

    template <typename T, typename... Args>
    static decltype(auto) addComponent(uint32_t entity, Args&&... args)
    {
        return ServiceRegistry::getInstance().getService<GameState>()->addComponent<T>(
            entity, std::forward<Args>(args)...);
    }

    template <typename T> static bool hasComponent(uint32_t entity)
    {
        return ServiceRegistry::getInstance().getService<GameState>()->hasComponent<T>(entity);
    }

    template <typename T> static T& getComponent(uint32_t entity)
    {
        return ServiceRegistry::getInstance().getService<GameState>()->getComponent<T>(entity);
    }

    template <typename... T> static decltype(auto) getComponents(uint32_t entity)
    {
        return ServiceRegistry::getInstance().getService<GameState>()->getComponents<T...>(entity);
    }

    template <typename... T> static auto getEntities()
    {
        return ServiceRegistry::getInstance().getService<GameState>()->getEntities<T...>();
    }
};
} // namespace ion

#endif