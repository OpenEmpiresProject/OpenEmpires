#ifndef FRAMEDATA_H
#define FRAMEDATA_H

#include "Feet.h"
#include "components/CompGraphics.h"

#include <vector>

namespace ion
{
struct FrameData
{
    int frameNumber = 0;
    std::vector<CompGraphics*> graphicUpdates; // Simulator to Renderer
    Vec2 viewportPositionInPixels;             // Renderer to simulator
};
} // namespace ion

#endif