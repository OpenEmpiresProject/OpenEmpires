#ifndef ZORDERSTRATEGYBASE_H
#define ZORDERSTRATEGYBASE_H

#include "Coordinates.h"
#include "components/CompRendering.h"

#include <vector>

namespace aion
{
class ZOrderStrategyBase
{
  public:
    virtual void preProcess(CompRendering& graphic)
    {
    }
    virtual const std::vector<CompRendering*>& zOrder(const Coordinates& coordinates) = 0;
    virtual ~ZOrderStrategyBase() = default;
};

} // namespace aion

#endif