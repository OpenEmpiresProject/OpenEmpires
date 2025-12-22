#include "TestWorldCreator.h"

#include "Coordinates.h"
#include "EntityFactory.h"
#include "EntityTypeRegistry.h"
#include "GameTypes.h"
#include "PlayerController.h"
#include "PlayerFactory.h"
#include "Renderer.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "SubSystemRegistry.h"
#include "Tile.h"
#include "UI.h"
#include "UIManager.h"
#include "commands/CmdIdle.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "logging/Logger.h"
#include "utils/ObjectPool.h"
#include "utils/Types.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_surface.h>
#include <SDL3_image/SDL_image.h>
#include <filesystem>
#include <memory>
#include <random>
#include <string>

#define WITH(statement) if (statement)

namespace fs = std::filesystem;
using namespace game;
using namespace core;
using namespace drs;
using namespace std;


void TestWorldCreator::create()
{
    spdlog::info("Creating the test world");

    InGameResource::registerResourceType(ResourceType::WOOD);
    InGameResource::registerResourceType(ResourceType::STONE);
    InGameResource::registerResourceType(ResourceType::GOLD);

    createTerrain();
    registerVillagerActions();
    createPlayers();
    registerVillagerActions();
    createHUD();
    createMiningCluster(getResourceType("gold"), 25, 25, 2);
    createTree(20, 25);

    m_isReady = true;

    spdlog::info("Test world created");
}
