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
        float speed = 10;
        bool repeatable = false;
        GraphicLayer layer = GraphicLayer::NONE;
    };
    Property<std::array<ActionAnimation, Constants::MAX_ANIMATIONS>> animations{};

  public:
    int frame = 0;
    GraphicLayer layer = GraphicLayer::NONE;
};
} // namespace core

#endif