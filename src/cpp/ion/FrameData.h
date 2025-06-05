#ifndef FRAMEDATA_H
#define FRAMEDATA_H

#include "Vec2d.h"
#include "components/CompGraphics.h"

#include <vector>

namespace ion
{
struct FrameData
{
    int frameNumber = 0;
    std::vector<CompGraphics*> graphicUpdates; // Simulator to Renderer
    Vec2d viewportPositionInPixels;            // Renderer to simulator
};
} // namespace ion

#endif