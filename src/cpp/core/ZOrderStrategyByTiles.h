#ifndef CORE_ZORDERSTRATEGYBYTILES_H
#define CORE_ZORDERSTRATEGYBYTILES_H

#include "Flat2DArray.h"
#include "Rect.h"
#include "TileMap.h"
#include "VersionedBucketSorter.h"
#include "ZOrderStrategy.h"

#include <queue>
#include <unordered_map>

namespace core
{
class ZOrderStrategyByTiles : public ZOrderStrategy
{
  public:
    ZOrderStrategyByTiles();
    ~ZOrderStrategyByTiles();

    void onUpdate(const CompRendering& current, CompRendering& graphic) override;
    const std::vector<CompRendering*>& zOrder(const Coordinates& coordinates) override;

  private:
    void processLayer(const MapLayerType& layer, const Coordinates& coordinates);
    void processUILayer();
    const std::vector<core::CompRendering*>& consolidateAllLayers();
    void resetAllLayers();
    bool isTileInsideExtendedViewport(const Rect<int>& extendedViewport,
                                      const Tile& tile,
                                      const Coordinates& coordinates) const;

  private:
    // Use to sort units inside a single tile
    struct WithinTileZBucket
    {
        int64_t version = 0;
        std::vector<CompRendering*> graphics;
    };

    TileMap m_gameMap;
    std::vector<CompRendering*> m_objectsToRenderByLayer[g_graphicLayersOrder.size()];
    std::vector<CompRendering*> m_finalListToRender;
    std::unordered_map<uint32_t, CompRendering*> m_uiGraphicsByEntity;
    VersionedBucketSorter<CompRendering*> m_uiElements;
    std::vector<CompRendering*> m_renderingComponents;
    std::vector<WithinTileZBucket> m_bucketsByZ;
    int64_t m_currentBucketVersion = 0;
    Ref<Settings> m_settings;
    Flat2DArray<uint64_t> m_alreadyProcessedTiles;
    uint32_t m_numberOfTilesProcessed = 0;

    const uint32_t m_extendedViewportVerticalMargin = 0;
    const uint32_t m_extendedViewportHorizontalMargin = 0;
};
} // namespace core

#endif // CORE_ZORDERSTRATEGYBYTILES_H
