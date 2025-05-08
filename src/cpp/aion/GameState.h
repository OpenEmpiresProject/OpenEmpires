#ifndef GAMESTATE_H
#define GAMESTATE_H

#include "PathFinderBase.h"
#include "StaticEntityMap.h"
#include "SubSystem.h"

#include <entt/entity/registry.hpp>

namespace aion
{
class GameState
{
  public:
    static GameState& getInstance()
    {
        static GameState instance;
        return instance;
    }

    GameState(const GameState&) = delete;
    GameState& operator=(const GameState&) = delete;

    uint32_t createEntity();
    void destroyEntity(uint32_t entity);
    bool isEntityValid(uint32_t entity) const;

    template <typename T, typename... Args>
    decltype(auto) addComponent(uint32_t entity, Args&&... args)
    {
        return registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
    }

    template <typename T> decltype(auto) addComponent(uint32_t entity, const T& t)
    {
        return registry.emplace_or_replace<T>(entity, t);
    }

    template <typename T> bool hasComponent(uint32_t entity) const
    {
        return registry.all_of<T>(entity);
    }

    template <typename T> T& getComponent(uint32_t entity)
    {
        return registry.get<T>(entity);
    }

    template <typename... T> decltype(auto) getComponents(uint32_t entity)
    {
        return registry.get<T...>(entity);
    }

    // write a function get all entities with all given components
    template <typename... T> auto getEntities()
    {
        return registry.view<T...>();
    }

    void clearAll();

    StaticEntityMap initializeStaticEntityMap(int width, int height);
    StaticEntityMap generateMap();
    StaticEntityMap generateDebugMap();

    StaticEntityMap staticEntityMap;

    PathFinderBase* getPathFinder() const
    {
        return pathFinder;
    }

  private:
    GameState();
    ~GameState();

    entt::basic_registry<uint32_t> registry;
    PathFinderBase* pathFinder = nullptr;
};
} // namespace aion

#endif