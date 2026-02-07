#ifndef COMP_ARMOR_H
#define COMP_ARMOR_H

#include "Property.h"

#include <vector>

namespace core
{
class CompArmor
{
  public:
    Property<std::vector<int>> armorPerClass;
    Property<float> damageResistance;
};

} // namespace core

#endif