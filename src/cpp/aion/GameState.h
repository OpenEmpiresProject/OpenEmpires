#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <entt/entity/registry.hpp>
#include "SubSystem.h"

namespace aion
{
    class GameState : public SubSystem
    {
    public:
        GameState(/* args */);
        ~GameState();
        
        entt::entity createEntity();
        void destroyEntity(entt::entity entity);
        bool isEntityValid(entt::entity entity) const;

        template<typename T, typename... Args>
        decltype(auto) addComponent(entt::entity entity, Args&&... args)
        {
            return registry.emplace_or_replace<T>(entity, std::forward<Args>(args)...);
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

        void clearAll();

    private:
        // SubSystem methods
        void init() override;
        void shutdown() override;
        
        entt::registry registry;
    };
}

#endif