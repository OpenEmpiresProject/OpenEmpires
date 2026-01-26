#ifndef COMPBUILDER_H
#define COMPBUILDER_H

#include "Property.h"

#include <cstdint>
#include <entt/entity/registry.hpp>

namespace core
{
class CompBuilder
{
  public:
    Property<uint32_t> buildSpeed;
    Property<std::unordered_map<char, uint32_t>> buildableTypesByShortcut;
    Property<std::unordered_map<char, std::string>> buildableNameByShortcut; // TODO - remove

  public:
    uint32_t target = entt::null;
};
} // namespace core

#endif