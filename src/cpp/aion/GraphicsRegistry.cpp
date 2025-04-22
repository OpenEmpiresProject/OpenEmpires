#include "GraphicsRegistry.h"

#include "Logger.h"

#include <stdexcept>

using namespace aion;

void aion::GraphicsRegistry::registerGraphic(const GraphicsID& graphicID,
                                             const GraphicsEntry& entry)
{
    auto it = graphicsMap.find(graphicID.hash());
    if (it != graphicsMap.end())
    {
        it->second = entry;
    }
    else
    {
        graphicsMap[graphicID.hash()] = entry;
    }
}

const GraphicsEntry& aion::GraphicsRegistry::getGraphic(const GraphicsID& graphicID) const
{
    // Check if the graphicID exists in the map
    auto it = graphicsMap.find(graphicID.hash());
    if (it == graphicsMap.end())
    {
        spdlog::error("Graphic ID not found in registry: {}", graphicID.toString());
        throw std::runtime_error("Graphic ID not found in registry:" + graphicID.toString());
    }
    return graphicsMap.at(graphicID.hash());
}
