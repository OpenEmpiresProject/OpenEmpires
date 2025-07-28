#ifndef RESOURCE_H
#define RESOURCE_H

#include "debug.h"
#include "utils/Constants.h"

#include <array>
#include <cstdint>

namespace ion
{
struct Resource
{
    uint8_t type = Constants::RESOURCE_TYPE_NONE;
    uint32_t amount = 0;

    Resource() = default;

    Resource(uint8_t type, uint32_t amount) : type(type), amount(amount)
    {
    }

    static void registerResourceType(uint8_t type)
    {
        debug_assert(type < Constants::MAX_RESOURCE_TYPES, "Invalid resource type {}", type);
        s_resourceTypes[type] = type;
    }

    static bool isResourceTypeValid(uint8_t type)
    {
        debug_assert(type < Constants::MAX_RESOURCE_TYPES, "Invalid resource type {}", type);
        return s_resourceTypes[type] != -1;
    }

  private:
    inline static std::array<int16_t, Constants::MAX_RESOURCE_TYPES> s_resourceTypes = []()
    {
        std::array<int16_t, Constants::MAX_RESOURCE_TYPES> arr;
        arr.fill(-1);
        return arr;
    }();
};

} // namespace ion

#endif