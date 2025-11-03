#ifndef ZORDERSTRATEGYBASE_H
#define ZORDERSTRATEGYBASE_H

#include "Coordinates.h"
#include "components/CompRendering.h"

#include <vector>

namespace core
{
/*
*  Implementations of this interface should respect/handle following conditions.
*   1. CompRendering's isEnabled and isDestoryed determine whether the entity is visible
*   2. CompRendering's positionInFeet can be null for UI elements
*   3. Should optimize to return only the entities within the viewport
*   4. Should entirely rely only on what is being informed via preProcess function (i.e.
*       can't access StateManager)
*   5. CompRendering's additionalZOffset should be added to the final z-order if present
*/
class ZOrderStrategy
{
  public:
    virtual void onUpdate(const CompRendering& current, CompRendering& graphic)
    {
    }
    virtual const std::vector<CompRendering*>& zOrder(const Coordinates& coordinates) = 0;
    virtual ~ZOrderStrategy() = default;
};

} // namespace core

#endif