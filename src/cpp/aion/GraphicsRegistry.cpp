#include "GraphicsRegistry.h"

#include "Logger.h"

#include <stdexcept>

using namespace aion;

void aion::GraphicsRegistry::registerTexture(const GraphicsID& graphicID, const Texture& entry)
{
    auto it = m_textureMap.find(graphicID.hash());
    if (it != m_textureMap.end())
    {
        it->second = entry;
    }
    else
    {
        m_textureMap[graphicID.hash()] = entry;
    }
}

const Texture& aion::GraphicsRegistry::getTexture(const GraphicsID& graphicID) const
{
    // Check if the graphicID exists in the map
    auto it = m_textureMap.find(graphicID.hash());
    if (it == m_textureMap.end())
    {
        spdlog::error("Graphic ID not found in registry: {}", graphicID.toString());
        throw std::runtime_error("Graphic ID not found in registry:" + graphicID.toString());
    }
    return m_textureMap.at(graphicID.hash());
}

void aion::GraphicsRegistry::registerAnimation(const GraphicsID& graphicID, const Animation& entry)
{
    if (graphicID.frame != 0)
    {
        spdlog::error("Animation ID should not have a frame value: {}", graphicID.toString());
        throw std::runtime_error("Animation ID should not have a frame value:" +
                                 graphicID.toString());
    }

    auto it = m_animationMap.find(graphicID.hash());
    if (it != m_animationMap.end())
    {
        it->second = entry;
    }
    else
    {
        m_animationMap[graphicID.hash()] = entry;
    }
}

const Animation& aion::GraphicsRegistry::getAnimation(const GraphicsID& graphicID) const
{
    int64_t hash = graphicID.hashWithClearingFrame();

    // Check if the graphicID exists in the map
    auto it = m_animationMap.find(hash);
    if (it == m_animationMap.end())
    {
        spdlog::error("Animation ID not found in registry: {}", graphicID.toString());
        throw std::runtime_error("Animation ID not found in registry:" + graphicID.toString());
    }
    return m_animationMap.at(hash);
}
