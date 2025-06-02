#include "GraphicsRegistry.h"

#include "utils/Logger.h"

#include <stdexcept>

using namespace aion;

void GraphicsRegistry::registerTexture(const GraphicsID& graphicID, const Texture& entry)
{
    // spdlog::debug("Registering texture with ID: {}", graphicID.toString());

    auto it = m_textureMap.find(graphicID.hash());
    if (it != m_textureMap.end())
    {
        spdlog::warn("Texture ID already exists, updating: {}", graphicID.toString());
        it->second = entry;
    }
    else
    {
        m_textureMap[graphicID.hash()] = entry;
    }

#ifndef NDEBUG
    m_graphicIds[graphicID.hash()] = graphicID;
    auto itEntityType = m_graphicIdsByEntityType.find(graphicID.entityType);
    if (itEntityType == m_graphicIdsByEntityType.end())
    {
        m_graphicIdsByEntityType[graphicID.entityType] = std::vector<GraphicsID>();
    }
    m_graphicIdsByEntityType[graphicID.entityType].push_back(graphicID);
#endif
}

const Texture& GraphicsRegistry::getTexture(const GraphicsID& graphicID) const
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

void GraphicsRegistry::registerAnimation(const GraphicsID& graphicID, const Animation& entry)
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
        spdlog::warn("Animation ID already exists, updating: {}", graphicID.toString());
        it->second = entry;
    }
    else
    {
        m_animationMap[graphicID.hash()] = entry;
    }
}

const Animation& GraphicsRegistry::getAnimation(const GraphicsID& graphicID) const
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

bool GraphicsRegistry::hasTexture(const GraphicsID& graphicID) const
{
    return m_textureMap.find(graphicID.hash()) != m_textureMap.end();
}

size_t GraphicsRegistry::getTextureCount() const
{
    return m_textureMap.size();
}

const std::unordered_map<int64_t, Texture>& GraphicsRegistry::getTextures() const
{
    return m_textureMap;
}

bool GraphicsRegistry::hasAnimation(const GraphicsID& graphicID) const
{
    int64_t hash = graphicID.hashWithClearingFrame();

    return m_animationMap.find(hash) != m_animationMap.end();
}

size_t GraphicsRegistry::getAnimationCount() const
{
    return m_animationMap.size();
}