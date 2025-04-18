#include "Entity.h"
#include "EntityFactory.h"

using namespace spice;

Entity::Entity(int type, int id, utils::Signature)
    : type(type), id(id), signature(signature) 
{
}

spice::Entity::~Entity()
{
    signature = 0; // Remove all components
}
