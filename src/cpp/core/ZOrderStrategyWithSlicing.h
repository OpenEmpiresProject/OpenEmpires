#ifndef ZORDERSTRATEGYWITHSLICING_H
#define ZORDERSTRATEGYWITHSLICING_H

#include "ZOrderStrategy.h"

#include <list>
#include <unordered_map>

namespace core
{
class ZOrderStrategyWithSlicing : public ZOrderStrategy
{
  public:
    ZOrderStrategyWithSlicing();
    ~ZOrderStrategyWithSlicing();

    void preProcess(CompRendering& graphic) override;
    const std::vector<CompRendering*>& zOrder(const Coordinates& coordinates) override;

  private:
    void addRenderingCompToZBuckets(CompRendering* rc, const Coordinates& coordinates);

  private:
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
    std::shared_ptr<Settings> m_settings;
    std::vector<CompRendering*> m_finalListToRender;
    std::vector<CompRendering*> m_objectsToRenderByLayer[g_graphicLayersOrder.size()];
};

} // namespace core

#endif