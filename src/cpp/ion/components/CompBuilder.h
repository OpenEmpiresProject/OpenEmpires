#ifndef COMPBUILDER_H
#define COMPBUILDER_H

#include <cstdint>
#include <entt/entity/registry.hpp>

namespace ion
{
class CompBuilder
{
  public:
    uint32_t buildSpeed = 0;
    uint32_t target = entt::null;

    CompBuilder(uint32_t buildSpeed) : buildSpeed(buildSpeed)
    {
    }
};
} // namespace ion

#endif