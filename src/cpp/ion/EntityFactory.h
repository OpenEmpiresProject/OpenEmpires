#ifndef ENTITYFACTORY_H
#define ENTITYFACTORY_H

#include <cstdint>

namespace ion
{
    class EntityFactory
    {
    public:
       virtual  ~EntityFactory() = default;

        virtual uint32_t createEntity(uint32_t entityType) = 0;
    };
} // namespace ion

#endif