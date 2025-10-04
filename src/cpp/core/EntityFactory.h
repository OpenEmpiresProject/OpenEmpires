#ifndef ENTITYFACTORY_H
#define ENTITYFACTORY_H

#include <cstdint>

namespace core
{
class EntityFactory
{
  public:
    virtual ~EntityFactory() = default;

    virtual uint32_t createEntity(uint32_t entityType, uint32_t entitySubType) = 0;
};
} // namespace core

#endif