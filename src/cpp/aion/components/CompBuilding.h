#ifndef COMPBUILDING_H
#define COMPBUILDING_H

#include "utils/Size.h"

namespace aion
{
class CompBuilding
{
  public:
    bool isPlacing = false;
    bool cantPlace = false;
    Size size{0, 0};
};

} // namespace aion

#endif