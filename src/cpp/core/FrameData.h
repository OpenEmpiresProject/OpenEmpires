#ifndef FRAMEDATA_H
#define FRAMEDATA_H

#include "Feet.h"
#include "FogOfWar.h"
#include "GraphicsRegistry.h"

#include <vector>

namespace core
{
class CompGraphics;

struct FrameData
{
    int frameNumber = 0;
    std::vector<CompGraphics*> graphicUpdates; // Simulator to Renderer
    // Even in a giant map (i.e. 240x240), fogOfWar information would take around 56KB
    // Therefore, it is totally acceptable to copy FogOfWar.
    FogOfWar fogOfWar;             // Simulator to Renderer
    GraphicsID cursor;             // Simulator to Renderer
    Vec2 viewportPositionInPixels; // Renderer to simulator
};
} // namespace core

#endif