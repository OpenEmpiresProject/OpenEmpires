#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>
#include <bitset>
#include "Types.h"

namespace spice
{
    class EntityFactory;
    class Entity
    {
        friend class EntityFactory; // Allow EntityFactory to create and destroy entities
    public:
        int getType() const { return type; }
        int getId() const { return id; }
        utils::Signature getSignature() const { return signature; }
        bool hasComponent(int componentType) const { return signature.test(componentType); }

    private:
        Entity() = default;
        Entity(int type, int id, utils::Signature);
        ~Entity();

        int type;
        int id;
        utils::Signature signature = 0;
    };
}


#endif