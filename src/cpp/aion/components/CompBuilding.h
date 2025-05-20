#ifndef COMPBUILDING_H
#define COMPBUILDING_H

#include "utils/Size.h"

namespace aion
{
class CompBuilding
{
  public:
    bool isPlacing = false;
    bool validPlacement = true;
    Size size{0, 0};
};

} // namespace aion

#endif