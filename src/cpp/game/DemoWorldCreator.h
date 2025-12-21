#ifndef DEMOWORLDCREATOR_H
#define DEMOWORLDCREATOR_H

#include "DRSFile.h"
#include "EventHandler.h"
#include "GameTypes.h"
#include "GraphicsRegistry.h"
#include "Player.h"
#include "Property.h"
#include "Renderer.h"
#include "Settings.h"
#include "StateManager.h"
#include "SubSystem.h"
#include "Tile.h"
#include "TileMap.h"
#include "WorldCreator.h"

#include <memory>

namespace game
{
class DemoWorldCreator : public WorldCreator, public core::PropertyInitializer
{
  public:
    struct Params : public WorldCreator::Params
    {
    };

    DemoWorldCreator(const Params& params);
    ~DemoWorldCreator() = default;

  protected:
    // EventHandler methods
    void onInit(core::EventLoop& eventLoop) override;
    bool isReady() const override;

    // WorldCreator methods
    void create() override;

    uint32_t getResourceType(const std::string& resourceName);
    void registerVillagerActions();
    void createTerrain();
    std::vector<core::Ref<core::Player>> createPlayers();

    void createHUD();
    void generateRandomForest();
    void createTile(uint32_t x, uint32_t y, EntityTypes entityType);
    void createTree(uint32_t x, uint32_t y);
    void createMiningCluster(uint32_t entityType, uint32_t xHint, uint32_t yHint, uint8_t amount);
    void createStoneOrGold(uint32_t entityType, uint32_t x, uint32_t y);
    void createVillager(core::Ref<core::Player> player, const core::Tile& pos);

    core::LazyServiceRef<core::StateManager> m_stateMan;
    bool m_isReady = false;

  private:
    core::LazyServiceRef<core::Settings> m_settings;
    const int m_iconSize = 36;
};
} // namespace game

#endif