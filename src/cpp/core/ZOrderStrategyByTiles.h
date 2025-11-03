#ifndef CORE_ZORDERSTRATEGYBYTILES_H
#define CORE_ZORDERSTRATEGYBYTILES_H

#include "ZOrderStrategy.h"
#include "TileMap.h"

#include <unordered_map>
#include <queue>

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
    //std::multimap<int, CompRendering*> m_uiLayerGraphics;
    std::unordered_map<uint32_t, CompRendering*> m_uiGraphicsByEntity;
    std::vector<CompRendering*> m_uiZBuckets;
    std::unordered_map<uint32_t, CompRendering*> m_renderingCompsByEntity;
    std::vector<WithinTileZBucket> m_bucketsByZ;
    int64_t m_currentBucketVersion = 0;
    Ref<Settings> m_settings;
};
} // namespace core

#endif // CORE_ZORDERSTRATEGYBYTILES_H
