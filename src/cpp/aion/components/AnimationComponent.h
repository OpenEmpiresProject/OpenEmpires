#ifndef ANIMATIONCOMPONENT_H
#define ANIMATIONCOMPONENT_H

#include "GraphicsRegistry.h"
#include "utils/WidthHeight.h"

#include <SDL3/SDL.h>

namespace aion
{
class AnimationComponent
{
  public:
    GraphicsID animationId; // GraphicsID to query GraphicsRegistry to get the Animation
};
} // namespace aion

#endif