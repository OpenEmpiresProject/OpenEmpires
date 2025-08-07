#include "GraphicsRegistry.h"

#include "utils/Logger.h"

#include <stdexcept>

using namespace core;

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

size_t GraphicsRegistry::getVariationCount(const GraphicsID& graphicID) const
{
    GraphicsID id = graphicID;

    for (int i = 0; i < 100; i++)
    {
        id.variation = i;
        if (!hasTexture(id))
        {
            return i;
        }
    }
    return 0;
}
