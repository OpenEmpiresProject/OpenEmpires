#ifndef COMP_ARMOR_H
#define COMP_ARMOR_H

#include "Property.h"

#include <unordered_map>
#include <vector>

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