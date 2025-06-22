#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include "DRSFile.h"
#include "GameSettings.h"
#include "GameState.h"
#include "GameTypes.h"
#include "GraphicsRegistry.h"
#include "Player.h"
#include "Renderer.h"
#include "SubSystem.h"
#include "Tile.h"
#include "TileMap.h"

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
    void generateMap(ion::TileMap& gameMap);
    void createTile(
        uint32_t x, uint32_t y, ion::GameState& gameState, EntityTypes entityType, bool isFOW);
    void createTree(ion::TileMap& gameMap, uint32_t x, uint32_t y);
    void createStoneOrGoldCluster(EntityTypes entityType,
                                  ion::TileMap& gameMap,
                                  uint32_t xHint,
                                  uint32_t yHint,
                                  uint8_t amount);
    void createStoneOrGold(EntityTypes entityType, ion::TileMap& gameMap, uint32_t x, uint32_t y);
    void createVillager(ion::Ref<ion::Player> player, const ion::Tile& pos);

    std::shared_ptr<ion::GameSettings> m_settings;
    ion::GraphicsRegistry& m_graphicsRegistry;
    std::shared_ptr<ion::Renderer> m_renderer;
    std::shared_ptr<drs::DRSFile> m_drs;
};
} // namespace game

#endif