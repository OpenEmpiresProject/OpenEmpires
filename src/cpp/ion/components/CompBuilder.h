#ifndef COMPBUILDER_H
#define COMPBUILDER_H

#include "Property.h"

#include <cstdint>
#include <entt/entity/registry.hpp>

namespace ion
{
class CompBuilder
{
  public:
    Property<uint32_t> buildSpeed;
    uint32_t target = entt::null;
};
} // namespace ion

#endif