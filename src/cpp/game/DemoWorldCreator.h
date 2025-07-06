#ifndef DEMOWORLDCREATOR_H
#define DEMOWORLDCREATOR_H

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
class DemoWorldCreator : public ion::SubSystem
{
  public:
    DemoWorldCreator(std::stop_token* stopToken,
                     std::shared_ptr<ion::GameSettings> settings,
                     ion::GraphicsRegistry& graphicsRegistry,
                     std::shared_ptr<ion::Renderer> renderer);
    ~DemoWorldCreator() = default;

  private:
    // SubSystem methods
    void init() override;
    void shutdown() override;

    void loadEntities();
    void generateMap(ion::TileMap& gameMap);
    void createTile(uint32_t x, uint32_t y, ion::GameState& gameState, EntityTypes entityType);
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