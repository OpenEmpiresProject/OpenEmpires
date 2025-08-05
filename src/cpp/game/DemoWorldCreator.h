#ifndef DEMOWORLDCREATOR_H
#define DEMOWORLDCREATOR_H

#include "DRSFile.h"
#include "GameSettings.h"
#include "GameState.h"
#include "GameTypes.h"
#include "GraphicsRegistry.h"
#include "Player.h"
#include "Property.h"
#include "Renderer.h"
#include "SubSystem.h"
#include "Tile.h"
#include "TileMap.h"

#include <memory>
namespace game
{
class DemoWorldCreator : public core::SubSystem, public core::PropertyInitializer
{
  public:
    DemoWorldCreator(std::stop_token* stopToken,
                     std::shared_ptr<core::GameSettings> settings,
                     bool populateWorld);
    ~DemoWorldCreator() = default;

    bool isReady() const;

  private:
    // SubSystem methods
    void init() override;
    void shutdown() override;

    void loadEntities();
    void generateMap(core::TileMap& gameMap);
    void createTile(uint32_t x,
                    uint32_t y,
                    core::Ref<core::GameState> gameState,
                    EntityTypes entityType);
    void createTree(core::TileMap& gameMap, uint32_t x, uint32_t y);
    void createStoneOrGoldCluster(EntityTypes entityType,
                                  core::TileMap& gameMap,
                                  uint32_t xHint,
                                  uint32_t yHint,
                                  uint8_t amount);
    void createStoneOrGold(EntityTypes entityType, core::TileMap& gameMap, uint32_t x, uint32_t y);
    void createVillager(core::Ref<core::Player> player, const core::Tile& pos);

    std::shared_ptr<core::GameSettings> m_settings;
    bool m_populateWorld = false;
    bool m_isReady = false;
};
} // namespace game

#endif