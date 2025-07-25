#ifndef ZORDERSTRATEGYWITHSLICING_H
#define ZORDERSTRATEGYWITHSLICING_H

#include "ZOrderStrategyBase.h"

#include <list>
#include <unordered_map>

namespace ion
{
class ZOrderStrategyWithSlicing : public ZOrderStrategyBase
{
  public:
    ZOrderStrategyWithSlicing();
    void preProcess(CompRendering& graphic) override;
    const std::vector<CompRendering*>& zOrder(const Coordinates& coordinates) override;
    ~ZOrderStrategyWithSlicing();

  private:
    void addRenderingCompToZBuckets(CompRendering* rc, const Coordinates& coordinates);

    struct ZBucketVersion
    {
        int64_t version = 0;
        std::vector<CompRendering*> graphicsComponents;
        std::list<CompRendering*> newGraphicsComponents;
    };
    std::list<CompRendering*> m_subRenderingComponents;
    std::unordered_map<uint32_t, std::list<CompRendering*>> m_subRenderingByEntityId;
    std::vector<ZBucketVersion> m_zBuckets;
    const size_t m_zBucketsSize = 0;
    int64_t m_zBucketVersion = 0;
    std::shared_ptr<GameSettings> m_settings;
    std::vector<CompRendering*> m_finalListToRender;
    std::vector<CompRendering*> m_objectsToRenderByLayer[GraphicLayersOrder.size()];
};

} // namespace ion

#endif