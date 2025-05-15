#ifndef COMPANIMATION_H
#define COMPANIMATION_H

#include "utils/Constants.h"

namespace aion
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

    int frame = 0;
    ActionAnimation animations[Constants::MAX_ANIMATIONS] = {};
};
} // namespace aion

#endif