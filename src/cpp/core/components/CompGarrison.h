#ifndef CORE_COMPGARRISON_H
#define CORE_COMPGARRISON_H

#include "Property.h"
#include "utils/Types.h"

#include <entt/entity/registry.hpp>
#include <unordered_set>
#include <vector>

namespace core
{
class CompGarrison
{
  public:
    Property<std::unordered_set<UnitType>> unitTypes;
    Property<uint32_t> capacity;

  public:
    struct GarrisonedUnit
    {
        uint32_t unitType = 0;
        uint32_t unitId = entt::null;

        GarrisonedUnit(uint32_t type, uint32_t id) : unitType(type), unitId(id)
        {
        }
    };
    std::vector<GarrisonedUnit> garrisonedUnits;
};
} // namespace core

#endif // CORE_COMPGARRISON_H
