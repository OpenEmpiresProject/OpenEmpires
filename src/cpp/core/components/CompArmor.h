#ifndef COMP_ARMOR_H
#define COMP_ARMOR_H

#include "Property.h"

#include <vector>
#include <unordered_map>

namespace core
{
class CompArmor
{
  public:
    Property<std::unordered_map<int, int>> armorPerClassMap;
    Property<std::vector<int>> armorPerClass;
    Property<float> damageResistance;
    Property<int> baseArmor;
};

} // namespace core

#endif