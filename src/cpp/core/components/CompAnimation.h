#ifndef COMPANIMATION_H
#define COMPANIMATION_H

#include "Property.h"
#include "utils/Constants.h"

namespace core
{
class CompAnimation
{
  public:
    struct ActionAnimation
    {
        int frames = 0;
        int speed = 10;
        bool repeatable = false;
    };
    Property<ActionAnimation> animations[Constants::MAX_ANIMATIONS] = {};

  public:
    int frame = 0;
};
} // namespace core

#endif