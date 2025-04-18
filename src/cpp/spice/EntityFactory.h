#ifndef ENTITYLIFECYCLEMANAGER_H
#define ENTITYLIFECYCLEMANAGER_H

#include <list>
#include "ObjectPool.h"
#include "Entity.h"
#include "EntityComponentMap.h"


namespace spice
{
    class EntityFactory
    {
    public:
        EntityFactory(const EntityComponentMap& entityComponentMap);
        ~EntityFactory();

        Entity* createEntity(int type);
        void destroyEntity(Entity* entity);

    private:
        utils::ObjectPool<Entity> entityPool;
        int nextEntityId = 0; // Used to generate unique entity IDs
        std::list<int> recycledIds; // List of recycled IDs for reuse
        const EntityComponentMap& entityComponentMap;
    };
}

#endif