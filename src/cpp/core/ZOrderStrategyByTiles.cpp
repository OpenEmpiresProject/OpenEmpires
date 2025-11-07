#include "ZOrderStrategyByTiles.h"

#include "ServiceRegistry.h"
#include "Settings.h"

#include <stack>
#include <unordered_set>

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

void pushBackGraphicToLayer(std::vector<CompRendering*>& layerObjects,
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

static Feet getBuildingAnchor(const Feet& center, const Size& size)
{
    const float halfWidth = (float) size.width / 2;
    const float halfHeight = (float) size.height / 2;

    return center +
           Feet(halfWidth * Constants::FEET_PER_TILE, halfHeight * Constants::FEET_PER_TILE) - 10;
}

constexpr auto g_allMapLayers =
    make_enum_array<MapLayerType, MapLayerType::GROUND, MapLayerType::MAX_LAYERS>();

ZOrderStrategyByTiles::ZOrderStrategyByTiles()
    : m_extendedViewportVerticalMargin(Constants::TILE_PIXEL_HEIGHT * 5),
      m_extendedViewportHorizontalMargin(Constants::TILE_PIXEL_WIDTH * 2)
{
    m_settings = ServiceRegistry::getInstance().getService<Settings>();

    m_gameMap.init(m_settings->getWorldSizeInTiles().width,
                   m_settings->getWorldSizeInTiles().height);

    m_finalListToRender.reserve(10000);
    for (size_t i = 0; i < g_graphicLayersOrder.size(); i++)
    {
        m_objectsToRenderByLayer[i].reserve(2000);
    }

    m_bucketsByZ.resize(Constants::FEET_PER_TILE * 3);
    m_uiZBuckets.resize(m_settings->getWindowDimensions().height);
    m_renderingComponents.resize(Constants::MAX_ENTITIES);
    m_alreadyProcessedTiles = Flat2DArray<uint64_t>(m_settings->getWindowDimensions().width,
                                                    m_settings->getWindowDimensions().height);
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
        const bool positionChanged =
            current.positionInFeet.toTile() != update.positionInFeet.toTile();
        Feet updatePosition = update.positionInFeet;
        Feet currentPosition = current.positionInFeet;

        if (update.isBig())
        {
            updatePosition = getBuildingAnchor(update.positionInFeet, size);   // Bottom tile
            currentPosition = getBuildingAnchor(current.positionInFeet, size); // Bottom tile
        }

        if (update.isDestroyed or not update.isEnabled)
        {
            m_gameMap.removeEntity(layer, currentPosition.toTile(), current.entityID, size);
        }
        else if (positionChanged)
        {
            if (current.entityID != entt::null)
                m_gameMap.removeEntity(layer, currentPosition.toTile(), current.entityID, size);

            m_gameMap.addEntity(layer, updatePosition.toTile(), update.entityID, size);
        }
    }
    else
    {
        if (update.layer != GraphicLayer::UI)
        {
            spdlog::error("All non-UI layers must have the position. {}", update.toString());
            return;
        }
        m_uiGraphicsByEntity[update.entityID] = &update;
    }
    m_renderingComponents[update.entityID] = &update;
}

const std::vector<core::CompRendering*>& ZOrderStrategyByTiles::zOrder(
    const Coordinates& coordinates)
{
    m_numberOfTilesProcessed = 0;
    resetAllLayers();

    for (auto mapLayer : g_allMapLayers)
        processLayer(mapLayer, coordinates);
    processUILayer();

    spam("Number of tiles processed in this frame is {}", m_numberOfTilesProcessed);

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

struct PendingEntityStartPosition
{
    Tile entityLeftTopCorner = Tile::null;
    uint32_t entityId = entt::null;
};

void ZOrderStrategyByTiles::processLayer(const MapLayerType& layer, const Coordinates& coordinates)
{
    static uint64_t iteration = 0;
    ++iteration;
    std::stack<PendingEntityStartPosition> pendingEntities;
    SDL_Rect viewportRect = {0, 0, m_settings->getWindowDimensions().width,
                             m_settings->getWindowDimensions().height};
    const Rect<int> extendedViewport(-m_extendedViewportHorizontalMargin,
                                     -m_extendedViewportVerticalMargin,
                                     viewportRect.w + (m_extendedViewportHorizontalMargin * 2),
                                     viewportRect.h + (m_extendedViewportVerticalMargin * 2));

    bool repositioned = false;

    for (int y = 0; y < m_gameMap.height; y++)
    {
        bool continueXAxis = true;
        repositioned = false;

        for (int x = 0; x < m_gameMap.width and continueXAxis; x++)
        {
            Tile tile(x, y);

            if (m_alreadyProcessedTiles.at(x, y) == iteration)
            {
                continue;
            }

            if (not isTileInsideExtendedViewport(extendedViewport, tile, coordinates))
            {
                continue;
            }

            ++m_currentBucketVersion;
            ++m_numberOfTilesProcessed;

            const auto& entities = m_gameMap.getEntities(layer, tile);
            const size_t numEntities = entities.size(); // This is O(1) since C++11

            for (auto entity : entities)
            {
                auto rc = m_renderingComponents[entity];
                if (rc == nullptr)
                {
                    spdlog::error("Unknown entity {}. Cannot happen.", entity);
                    continue;
                }

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
                        auto& graphicsLayer =
                            m_objectsToRenderByLayer[toInt(getGraphicLayer(layer))];
                        pushBackGraphicToLayer(graphicsLayer, rc, viewportRect, coordinates);

                        // Taking left bottom (logically) corner as anchor and marking rest of
                        // the land size as already considered
                        //
                        for (uint32_t i = 0; i < rc->landSize.width; i++)
                        {
                            for (uint32_t j = 0; j < rc->landSize.height; j++)
                            {
                                m_alreadyProcessedTiles.at((uint64_t) x + i, (uint64_t) y - j) =
                                    iteration;
                            }
                        }

                        if (not pendingEntities.empty())
                        {
                            const auto& pending = pendingEntities.top();
                            // -1 is to let the loop increment adjust it back
                            x = pending.entityLeftTopCorner.x - 1;
                            y = pending.entityLeftTopCorner.y - 1;
                            pendingEntities.pop();
                            repositioned = true;

                            break; // break from entities, x break will be handled outside
                        }
                    }
                    else
                    {
                        // Put the entity to pending to visit later once its dependencies are
                        // resolved, but only if it is not already tracked.
                        if (pendingEntities.empty() or pendingEntities.top().entityId != entity)
                        {
                            pendingEntities.push(PendingEntityStartPosition{Tile(x, y), entity});
                        }
                        // Either way dependencies are not resolved, so can't continue to process
                        // remaining of Xs
                        continueXAxis = false;
                        break;
                    }
                }
                else
                {
                    if (numEntities == 1)
                    {
                        auto& graphicsLayer =
                            m_objectsToRenderByLayer[toInt(getGraphicLayer(layer))];
                        pushBackGraphicToLayer(graphicsLayer, rc, viewportRect, coordinates);
                    }
                    else
                    {
                        int withinTileX =
                            rc->positionInFeet.x - rc->positionInFeet.toTile().toFeet().x;
                        int withinTileY =
                            rc->positionInFeet.y - rc->positionInFeet.toTile().toFeet().y;
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
                    }
                    m_alreadyProcessedTiles.at(x, y) = iteration;
                }
            }

            if (repositioned)
                break; // breaks x

            if (numEntities > 1)
            {
                for (int z = 0; z < m_bucketsByZ.size(); ++z)
                {
                    if (m_bucketsByZ[z].version != m_currentBucketVersion)
                    {
                        continue; // Bucket wasn't updated for the current tile
                    }

                    for (auto& rc : m_bucketsByZ[z].graphics)
                    {
                        auto& graphicsLayer =
                            m_objectsToRenderByLayer[toInt(getGraphicLayer(layer))];
                        pushBackGraphicToLayer(graphicsLayer, rc, viewportRect, coordinates);
                    }
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

bool ZOrderStrategyByTiles::isTileInsideExtendedViewport(const Rect<int>& extendedViewport,
                                                         const Tile& tile,
                                                         const Coordinates& coordinates) const
{
    const auto tileCenter = tile.centerInFeet();
    const auto screenPos = coordinates.feetToScreenUnits(tileCenter);

    return extendedViewport.x <= screenPos.x and
           screenPos.x < (extendedViewport.x + extendedViewport.w) and
           extendedViewport.y <= screenPos.y and
           screenPos.y < (extendedViewport.y + extendedViewport.h);
}
