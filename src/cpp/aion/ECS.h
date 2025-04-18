#ifndef ECSSUBSYSTEM_H
#define ECSSUBSYSTEM_H

#include "SubSystem.h"
#include "EntityComponentMap.h"
#include "EntityFactory.h"
#include "ComponentStorage.h"

namespace aion
{
    class ECS : public SubSystem
    {
    public:
        ECS();
        ~ECS() override = default;

        void init() override
        {
            // Initialize the ECS subsystem
        }

        void shutdown() override
        {
            // Shutdown the ECS subsystem
        }

        template <typename ComponentType>
        void registerComponent()
        {
            componentStorage.registerComponent<ComponentType>();
        }

        template <typename ComponentType>
        void addComponent(int entityType, const ComponentType& component)
        {
            // Component has to be introduced before adding it to an entity
            if (componentStorage.hasComponent(ComponentType::type()))
            {
                entityComponentMap.addComponent(entityType, ComponentType::type());
            }
            else
            {
                // TODO: Throw error
            }
        }

        template <typename ComponentType>
        bool hasComponent(int entityType)
        {
            return entityComponentMap.hasComponent(entityType, ComponentType::type());
        }

        bool hasComponent(int entityType, int componentType)
        {
            return entityComponentMap.hasComponent(entityType, componentType);
        }

        spice::Entity* createEntity(int entityType)
        {
            auto entity = entityFactory.createEntity(entityType);

            if (entitiesByType.find(entityType) == entitiesByType.end())
            {
                entitiesByType[entityType] = std::unordered_map<int, spice::Entity*>();
            }
            entitiesByType[entityType][entity->getId()] = entity;
            return entity;
        }

        void destroyEntity(spice::Entity* entity)
        {
            entitiesByType[entity->getType()].erase(entity->getId());
            entityFactory.destroyEntity(entity);
        }

        class EntityIterator
        {
        public:
            EntityIterator(const std::unordered_map<int, spice::Entity*>& entities) : entities(entities) {}

            auto begin() { return entities.begin(); }
            auto end() { return entities.end(); }

        private:
            const std::unordered_map<int, spice::Entity*>& entities;
        };

        EntityIterator getEntitiesByType(int entityType)
        {
            // TODO: Check if entityType is valid
            auto it = entitiesByType.find(entityType);
            if (it != entitiesByType.end())
            {
                return EntityIterator(it->second);
            }
            return EntityIterator(std::unordered_map<int, spice::Entity*>());
        }   

    private:
        spice::EntityComponentMap entityComponentMap;
        spice::EntityFactory entityFactory;
        spice::ComponentStorage componentStorage;
        
        // <entityType, <entityID, list of entities>>
        std::unordered_map<int, std::unordered_map<int, spice::Entity*>> entitiesByType;
    };
    
} // namespace aion


#endif