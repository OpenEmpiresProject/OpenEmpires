#include "ZOrderStrategyByTiles.h"
#include "ServiceRegistry.h"
#include "Settings.h"
#include <unordered_set>
#include <stack>

using namespace core;

constexpr MapLayerType getMapLayerType(GraphicLayer graphicsLayer) noexcept
{
    switch (graphicsLayer)
    {
    case GraphicLayer::GROUND:
        return MapLayerType::GROUND;
    case GraphicLayer::ON_GROUND:
        return MapLayerType::ON_GROUND;
    case GraphicLayer::ENTITIES:
        return MapLayerType::STATIC;
    }
    return MapLayerType::STATIC;
}

constexpr GraphicLayer getGraphicLayer(MapLayerType mapLayerType) noexcept
{
    switch (mapLayerType)
    {
    case MapLayerType::GROUND:
        return GraphicLayer::GROUND;
    case MapLayerType::ON_GROUND:
        return GraphicLayer::ON_GROUND;
    case MapLayerType::STATIC:
        return GraphicLayer::ENTITIES;
    }
    return GraphicLayer::ENTITIES;
}

void addGraphicToBackOfLayer(std::vector<CompRendering*>& layerObjects,
                             CompRendering* graphic,
                             const SDL_Rect& viewport,
                             const Coordinates& coordinates)
{
    if (not graphic->positionInFeet.isNull())
    {
        auto screenPos = coordinates.feetToScreenUnits(graphic->positionInFeet) - graphic->anchor;

        SDL_Rect dstRectInt = {screenPos.x, screenPos.y, graphic->srcRect.w, graphic->srcRect.h};

        // Skip any texture that doesn't overlap with viewport (i.e. outside of screen)
        // This has reduced frame rendering time from 6ms to 3ms for 1366x768 window on the
        // development setup for 50x50 map in debug mode on Windows.
        if (!SDL_HasRectIntersection(&viewport, &dstRectInt))
        {
            return;
        }
    }

    layerObjects.emplace_back(graphic);
}

constexpr auto g_allMapLayers =
    make_enum_array<MapLayerType, MapLayerType::GROUND, MapLayerType::MAX_LAYERS>();

ZOrderStrategyByTiles::ZOrderStrategyByTiles()
{
    m_settings = ServiceRegistry::getInstance().getService<Settings>();

    m_gameMap.init(m_settings->getWorldSizeInTiles().width,
                   m_settings->getWorldSizeInTiles().height);

    m_finalListToRender.reserve(10000);
    for (size_t i = 0; i < g_graphicLayersOrder.size(); i++)
    {
        m_objectsToRenderByLayer[i].reserve(2000);
    }

    m_bucketsByZ.resize(Constants::FEET_PER_TILE * 10);
    m_uiZBuckets.resize(m_settings->getWindowDimensions().height);
}

ZOrderStrategyByTiles::~ZOrderStrategyByTiles()
{
    // destructor
}

// Approach: Lookup previous position from the local cache and use that
// and new update to maintain local game map copy. Then when it comes to 
// z-ordering, game map can be used order.
//
void ZOrderStrategyByTiles::onUpdate(const CompRendering& current, CompRendering& update)
{
    if (not update.positionInFeet.isNull())
    {
        const auto& layer = getMapLayerType(update.layer);
        Size size = update.isBig() ? update.landSize : Size(1, 1);

        if (current.positionInFeet.toTile() != update.positionInFeet.toTile())
            m_gameMap.removeEntity(layer, current.positionInFeet.toTile(), current.entityID, size);

        m_gameMap.addEntity(layer, update.positionInFeet.toTile(), update.entityID, size);
    }
    else
    {
        if (update.layer != GraphicLayer::UI)
        {
            spdlog::error("All non-UI layers must have the position. {}",
                          update.toString());
            return;
        }
        m_uiGraphicsByEntity[update.entityID] = &update;
    }
    m_renderingCompsByEntity[update.entityID] = &update;
}

const std::vector<core::CompRendering*>& ZOrderStrategyByTiles::zOrder(
    const Coordinates& coordinates)
{
    resetAllLayers();

    for (auto mapLayer : g_allMapLayers)
        processLayer(mapLayer, coordinates);
    processUILayer();
    
    return consolidateAllLayers();
}

void ZOrderStrategyByTiles::resetAllLayers()
{
    m_finalListToRender.clear();

    for (size_t i = 0; i < g_graphicLayersOrder.size(); i++)
    {
        m_objectsToRenderByLayer[i].clear();
    }
}

const std::vector<core::CompRendering*>& ZOrderStrategyByTiles::consolidateAllLayers()
{
    for (size_t i = 0; i < g_graphicLayersOrder.size(); i++)
    {
        m_finalListToRender.insert(m_finalListToRender.end(), m_objectsToRenderByLayer[i].begin(),
                                   m_objectsToRenderByLayer[i].end());
    }
    return m_finalListToRender;
}

void ZOrderStrategyByTiles::processLayer(const MapLayerType& layer, const Coordinates& coordinates)
{
    std::unordered_set<int> alreadyOrderedTiles;
    std::stack<std::pair<int, int>> pendingY;
    SDL_Rect viewportRect = {0, 0, m_settings->getWindowDimensions().width,
                             m_settings->getWindowDimensions().height};

    for (int y = 0; y < m_gameMap.height; y++)
    {
        bool continueXAxis = true;
        for (int x = 0; x < m_gameMap.width and continueXAxis; x++)
        {
            const int tileId = x + y * m_gameMap.width;
            if (alreadyOrderedTiles.contains(tileId))
                continue;

            ++m_currentBucketVersion;

            Tile tile(x, y);
            const auto& entities = m_gameMap.getEntities(layer, tile);

            for (auto entity : entities)
            {
                auto rc = m_renderingCompsByEntity[entity];

                if (rc->isDestroyed || rc->isEnabled == false)
                    continue;

                if (rc->isBig())
                {
                    Tile nextYTile(x, y + 1);
                    bool isCornerOfEntity = false;

                    if (m_gameMap.isValidPos(nextYTile))
                    {
                        auto nextYEntity = m_gameMap.getEntity(layer, nextYTile);
                        if (entity != nextYEntity)
                        {
                            // Different entity, so we are at the corner of the entity
                            isCornerOfEntity = true;
                        }
                    }
                    else
                    {
                        isCornerOfEntity = true;
                    }

                    if (isCornerOfEntity)
                    {
                        // std::cout<< fmt::format("Found building {} corner at {}", entity,
                        //               tile.toString())<< std::endl;
                        auto& graphicsLayer =
                            m_objectsToRenderByLayer[toInt(getGraphicLayer(layer))];
                        addGraphicToBackOfLayer(graphicsLayer, rc, viewportRect, coordinates);
                        

                        // Taking left bottom (logically) corner as anchor and marking rest of
                        // the land size as already considered
                        //
                        for (int i = 0; i < rc->landSize.width; i++)
                        {
                            for (int j = 0; j < rc->landSize.height; j++)
                            {
                                const int sameEntityTileId = (x + i) + (y - j) * m_gameMap.width;
                                alreadyOrderedTiles.insert(sameEntityTileId);
                            }
                        }

                        if (not pendingY.empty())
                        {
                            auto [newY, newX] = pendingY.top();
                            x = newX - 1; // -1 is to let the loop increment adjust it back
                            y = newY - 1;
                            pendingY.pop();

                            // std::cout << fmt::format("Switching pending Y of entity {}
                            // at{},{}",
                            //     entity, newX, newY)
                            //           << std::endl;
                        }
                    }
                    else
                    {
                        // std::cout << fmt::format("Not a corner for building {} at {}",
                        // entity,
                        //                          tile.toString())
                        //           << std::endl;
                        //  Still the same entity, will complete rest of x cells later. For now,
                        //  move to next row (i.e. y)
                        //
                        pendingY.push(std::make_pair(y, x));
                        continueXAxis = false;
                        break;
                    }
                }
                else
                {
                    // std::cout << fmt::format("Not a big entity {} at {}", entity,
                    //                          tile.toString())
                    //           << std::endl;

                    int withinTileX = rc->positionInFeet.x - rc->positionInFeet.toTile().toFeet().x;
                    int withinTileY = rc->positionInFeet.y - rc->positionInFeet.toTile().toFeet().y;
                    int zOrder = withinTileX + withinTileY + rc->additionalZOffset;

                    if (zOrder < 0 or zOrder >= m_bucketsByZ.size())
                    {
                        spdlog::error("Z-order out of bounds: {}", zOrder);
                        continue;
                    }

                    if (m_currentBucketVersion != m_bucketsByZ[zOrder].version)
                    {
                        m_bucketsByZ[zOrder].version = m_currentBucketVersion;
                        m_bucketsByZ[zOrder].graphics.clear();
                    }
                    m_bucketsByZ[zOrder].graphics.push_back(rc);
                    alreadyOrderedTiles.insert(tileId);
                }
            }

            for (int z = 0; z < m_bucketsByZ.size(); ++z)
            {
                if (m_bucketsByZ[z].version != m_currentBucketVersion)
                {
                    continue; // Bucket wasn't updated for the current tile
                }

                for (auto& rc : m_bucketsByZ[z].graphics)
                {
                    //m_objectsToRenderByLayer[toInt(getGraphicLayer(layer))].emplace_back(rc);
                    auto& graphicsLayer = m_objectsToRenderByLayer[toInt(getGraphicLayer(layer))];
                    addGraphicToBackOfLayer(graphicsLayer, rc, viewportRect, coordinates);
                }
            }
        }
    }
}

void ZOrderStrategyByTiles::processUILayer()
{
    std::fill(m_uiZBuckets.begin(), m_uiZBuckets.end(), nullptr);

    for (auto& [_, graphic] : m_uiGraphicsByEntity)
    {
        m_uiZBuckets[graphic->positionInScreenUnits.y] = graphic;
    }

    for (auto sortedUIItem : m_uiZBuckets)
    {
        if (sortedUIItem != nullptr)
        {
            m_objectsToRenderByLayer[toInt(GraphicLayer::UI)].emplace_back(sortedUIItem);
        }
    }
}

