#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <entt/entity/registry.hpp>
#include "SubSystem.h"

namespace aion
{
    class GameState : public SubSystem
    {
    public:
        static GameState& getInstance()
        {
            static GameState instance;
            return instance;
        }

        GameState(const GameState&) = delete;
        GameState& operator=(const GameState&) = delete;

        entt::entity createEntity();
        void destroyEntity(entt::entity entity);
        bool isEntityValid(entt::entity entity) const;

        template<typename T, typename... Args>
        decltype(auto) addComponent(entt::entity entity, Args&&... args)
        {
            return registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
        }

        template<typename T>
        decltype(auto) addComponent(entt::entity entity, const T& t)
        {
            return registry.emplace_or_replace<T>(entity, t);
        }

        template<typename T>
        bool hasComponent(entt::entity entity) const
        {
            return registry.all_of<T>(entity);
        }

        template<typename T>
        T& getComponent(entt::entity entity)
        {
            return registry.get<T>(entity);
        }

        template<typename... T>
        decltype(auto) getComponents(entt::entity entity)
        {
            return registry.get<T...>(entity);
        }

        // write a function to get all entities with a specific component
        template<typename T>
        auto getEntities()
        {
            return registry.view<T>();
        }

        void clearAll();

    private:
        GameState();
        ~GameState();

        // SubSystem methods
        void init() override;
        void shutdown() override;
        
        entt::registry registry;
    };
}

#endif