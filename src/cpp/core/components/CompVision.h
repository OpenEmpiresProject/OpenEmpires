#ifndef CORE_COMPVISION_H
#define CORE_COMPVISION_H

#include "Property.h"
#include "utils/Types.h"

#include <unordered_set>

namespace core
{
class CompVision
{
  public:
    Property<uint32_t> lineOfSight; // In Feet
    Property<LineOfSightShape> lineOfSightShape;
    Property<bool> activeTracking;

  public:
    Property<std::string> lineOfSightShapeStr;

  public:
    bool hasVision = false; // Dynamically control whether LOS is in effect
    std::unordered_set<uint32_t> nearbyEntities;
};
} // namespace core

#endif // CORE_COMPVISION_H
