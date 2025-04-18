#include "EntityComponentMap.h"

using namespace spice;

void EntityComponentMap::addComponent(int entityType, int componentType)
{
    entityComponentMap[entityType].set(componentType);
}

void EntityComponentMap::removeComponent(int entityType, int componentType)
{
    entityComponentMap[entityType].reset(componentType);
}

bool EntityComponentMap::hasComponent(int entityType, int componentType) const
{
    auto it = entityComponentMap.find(entityType);
    if (it != entityComponentMap.end())
    {
        return it->second.test(componentType);
    }
    return false;
}

utils::Signature spice::EntityComponentMap::getSignature(int entityType) const
{
    return entityComponentMap.at(entityType);
}

void spice::EntityComponentMap::clearSignature(int entityType)
{
    entityComponentMap.erase(entityType);
}

void spice::EntityComponentMap::clear()
{
    entityComponentMap.clear();
}
