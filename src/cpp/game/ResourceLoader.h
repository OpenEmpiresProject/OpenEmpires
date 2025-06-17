#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include "DRSFile.h"
#include "GameSettings.h"
#include "GameState.h"
#include "GameTypes.h"
#include "GraphicsRegistry.h"
#include "GridMap.h"
#include "Renderer.h"
#include "SubSystem.h"

#include <memory>
namespace game
{
class ResourceLoader : public ion::SubSystem
{
  public:
    ResourceLoader(std::stop_token* stopToken,
                   std::shared_ptr<ion::GameSettings> settings,
                   ion::GraphicsRegistry& graphicsRegistry,
                   std::shared_ptr<ion::Renderer> renderer);
    ~ResourceLoader() = default;

  private:
    // SubSystem methods
    void init() override;
    void shutdown() override;

    void loadEntities();
    void generateMap(ion::GridMap& gameMap);
    void createTree(ion::GridMap& gameMap, uint32_t x, uint32_t y);
    void createStoneOrGoldCluster(EntityTypes entityType,
                                  ion::GridMap& gameMap,
                                  uint32_t xHint,
                                  uint32_t yHint,
                                  uint8_t amount);
    void createStoneOrGold(EntityTypes entityType, ion::GridMap& gameMap, uint32_t x, uint32_t y);

    std::shared_ptr<ion::GameSettings> m_settings;
    ion::GraphicsRegistry& m_graphicsRegistry;
    std::shared_ptr<ion::Renderer> m_renderer;
    std::shared_ptr<drs::DRSFile> m_drs;
};
} // namespace game

#endif