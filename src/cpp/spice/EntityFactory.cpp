#include "EntityFactory.h"
#include "Constants.h"

using namespace spice;

EntityFactory::EntityFactory(const EntityComponentMap& entityComponentMap) 
    : entityComponentMap(entityComponentMap) 
{
    entityPool.reserve(utils::Constants::MAX_ENTITIES);
}

spice::EntityFactory::~EntityFactory()
{
    while (entityPool.getFreeSize() > 0)
    {
        delete entityPool.acquire();
    }
}

Entity* EntityFactory::createEntity(int type)
{
    int id = 0;
    if (!recycledIds.empty())
    {
        // Reuse an ID from the recycled list
        id = recycledIds.front();
        recycledIds.pop_front();
    }
    else
    {
        if (nextEntityId >= utils::Constants::MAX_ENTITIES)
        {
            // TODO: Handle error: maximum number of entities reached
            return nullptr;
        }
        
        id = nextEntityId++;
    }
    
    // FIX: Doesn't work. Type and id aren't initialized. 
    auto entity = entityPool.acquire();

    new (entity) Entity(type, id, entityComponentMap.getSignature(type)); // Placement new to construct the entity in the allocated memory
}

void EntityFactory::destroyEntity(Entity* entity)
{
    recycledIds.push_back(entity->getId());
    entityPool.release(entity);
}
