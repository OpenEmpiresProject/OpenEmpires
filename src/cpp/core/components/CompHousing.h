#ifndef CORE_COMPHOUSING_H
#define CORE_COMPHOUSING_H

#include "Property.h"

namespace core
{
class CompHousing
{
  public:
    Property<uint32_t> housingCapacity;

  public:
    static constexpr auto properties()
    {
        return std::tuple{
            PropertyDesc<&CompHousing::housingCapacity>{"Housing", "housing_capacity"}};
    }
};
} // namespace core

#endif // CORE_COMPHOUSING_H
