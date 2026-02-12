#ifndef COMP_ATTACK_H
#define COMP_ATTACK_H

#include "Property.h"

#include <vector>

namespace core
{
class CompAttack
{
  public:
    Property<std::vector<int>> attackPerClass;
    Property<std::vector<float>> attackMultiplierPerClass;
    Property<int> attackRate;
};

} // namespace core

#endif