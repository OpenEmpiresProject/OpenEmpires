#ifndef ENTITYCOMPONENTMAP_H
#define ENTITYCOMPONENTMAP_H

#include <unordered_map>
#include <bitset>
#include <memory>
#include <mutex>
#include "Types.h"

namespace spice
{
    // EntityComponentMap is a map that associates entity types with component types.
    // It allows for efficient storage and retrieval of components associated with entities.
    class EntityComponentMap
    {
    public:
        EntityComponentMap() = default;
        ~EntityComponentMap() = default;

        void addComponent(int entityType, int componentType);
        void removeComponent(int entityType, int componentType);
        bool hasComponent(int entityType, int componentType) const;
        utils::Signature getSignature(int entityType) const;
        void clearSignature(int entityType);
        void clear();

    private:
        std::unordered_map<int, utils::Signature> entityComponentMap;
    };

}

#endif