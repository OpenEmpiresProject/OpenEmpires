#include "GameAPI.h"

#include "GameTypes.h"
#include "PlayerManager.h"
#include "Renderer.h"
#include "ServiceRegistry.h"
#include "SubSystemRegistry.h"
#include "commands/CmdIdle.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"

using namespace game;
using namespace ion;

struct ScopedSynchronizer
{
    ScopedSynchronizer(std::shared_ptr<GameAPI::Synchronizer> synchronizer)
        : m_synchronizer(synchronizer)
    {
        if (m_synchronizer)
            m_synchronizer->onStart();
    }

    ~ScopedSynchronizer()
    {
        if (m_synchronizer)
            m_synchronizer->onEnd();
    }

  private:
    std::shared_ptr<GameAPI::Synchronizer> m_synchronizer;
};

bool GameAPI::isReady()
{
    auto redndererSubSystem = SubSystemRegistry::getInstance().getSubSystem("Renderer");
    auto renderer = (Renderer*) redndererSubSystem;
    return renderer->getSDLRenderer() != nullptr;
}

void GameAPI::quit()
{
    SDL_Event* event = new SDL_Event();
    event->type = SDL_EVENT_QUIT;
    SDL_PushEvent(event);
}

Ref<Player> GameAPI::getPrimaryPlayer()
{
    ScopedSynchronizer sync(m_sync);

    auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
    return playerManager->getViewingPlayer();
}

uint32_t GameAPI::createVillager(Ref<ion::Player> player, const Feet& pos)
{
    ScopedSynchronizer sync(m_sync);

    auto tilePos = pos.toTile();
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto villager = gameState->createEntity();
    auto transform = CompTransform(tilePos.x * 256 + 128, tilePos.x * 256 + 50);
    transform.face(Direction::SOUTH);
    transform.hasRotation = true;
    transform.speed = 256;

    CompAnimation anim;
    anim.animations[UnitAction::IDLE].frames = 15;
    anim.animations[UnitAction::IDLE].repeatable = true;
    anim.animations[UnitAction::IDLE].speed = 10;

    anim.animations[UnitAction::MOVE].frames = 15;
    anim.animations[UnitAction::MOVE].repeatable = true;
    anim.animations[UnitAction::MOVE].speed = 15;

    anim.animations[UnitAction::CHOPPING].frames = 15;
    anim.animations[UnitAction::CHOPPING].repeatable = true;
    anim.animations[UnitAction::CHOPPING].speed = 15;

    anim.animations[UnitAction::MINING].frames = 15;
    anim.animations[UnitAction::MINING].repeatable = true;
    anim.animations[UnitAction::MINING].speed = 15;

    anim.animations[UnitAction::CARRYING_LUMBER].frames = 15;
    anim.animations[UnitAction::CARRYING_LUMBER].repeatable = true;
    anim.animations[UnitAction::CARRYING_LUMBER].speed = 15;

    anim.animations[UnitAction::CARRYING_GOLD].frames = 15;
    anim.animations[UnitAction::CARRYING_GOLD].repeatable = true;
    anim.animations[UnitAction::CARRYING_GOLD].speed = 15;

    anim.animations[UnitAction::CARRYING_STONE].frames = 15;
    anim.animations[UnitAction::CARRYING_STONE].repeatable = true;
    anim.animations[UnitAction::CARRYING_STONE].speed = 15;

    anim.animations[UnitAction::BUILDING].frames = 15;
    anim.animations[UnitAction::BUILDING].repeatable = true;
    anim.animations[UnitAction::BUILDING].speed = 25;

    gameState->addComponent(villager, transform);
    gameState->addComponent(villager, CompRendering());
    gameState->addComponent(villager, CompEntityInfo(3));
    gameState->addComponent(villager, CompAction(0));
    gameState->addComponent(villager, anim);
    gameState->addComponent(villager, CompDirty());
    gameState->addComponent(villager, CompBuilder(10));

    // villager goes idle by default
    CompUnit unit;
    unit.lineOfSight = 256 * 5;
    unit.commandQueue.push(ObjectPool<CmdIdle>::acquire(villager));
    gameState->addComponent(villager, unit);

    CompGraphics gc;
    gc.entityID = villager;
    gc.entityType = EntityTypes::ET_VILLAGER;
    gc.layer = GraphicLayer::ENTITIES;
    gc.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::GREEN,
                                DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                DebugOverlay::FixedPosition::CENTER});
    gc.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::RED,
                                DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                DebugOverlay::FixedPosition::CENTER});
    gc.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::BLUE,
                                DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                DebugOverlay::FixedPosition::CENTER});
    gc.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::YELLOW,
                                DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                DebugOverlay::FixedPosition::CENTER});
    gc.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::BLACK,
                                DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                DebugOverlay::FixedPosition::CENTER});
    gameState->addComponent(villager, gc);

    CompSelectible sc;
    // auto box = getBoundingBox(m_drs, 1388, 1);
    // sc.boundingBoxes[static_cast<int>(Direction::NONE)] = box;
    sc.selectionIndicator = {GraphicAddon::Type::ISO_CIRCLE,
                             GraphicAddon::IsoCircle{10, Vec2(0, 0)}};

    gameState->addComponent(villager, sc);
    gameState->addComponent(villager, CompPlayer{player});

    CompResourceGatherer gatherer{
        .capacity = 100,
    };
    gameState->addComponent(villager, gatherer);

    auto newTile = transform.position.toTile();
    gameState->gameMap.addEntity(MapLayerType::UNITS, newTile, villager);

    player->getFogOfWar()->markAsExplored(transform.position, unit.lineOfSight);

    return villager;
}

std::list<uint32_t> GameAPI::getVillagers()
{
    ScopedSynchronizer sync(m_sync);

    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    std::list<uint32_t> villagers;

    gameState->getEntities<CompBuilder>().each([&villagers](uint32_t entity, CompBuilder&)
                                               { villagers.push_back(entity); });
    return villagers;
}

void GameAPI::commandToMove(uint32_t unit, const Feet& target)
{
    ScopedSynchronizer sync(m_sync);

    // TODO
}

int GameAPI::getCurrentAction(uint32_t unit)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto& action = gameState->getComponent<CompAction>(unit);
    return action.action;
}
