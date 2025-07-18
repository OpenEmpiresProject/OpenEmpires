#include "GameAPI.h"

#include "DemoWorldCreator.h"
#include "EventLoop.h"
#include "EventPublisher.h"
#include "GameTypes.h"
#include "PlayerManager.h"
#include "Renderer.h"
#include "ServiceRegistry.h"
#include "SubSystemRegistry.h"
#include "commands/CmdIdle.h"
#include "commands/CmdMove.h"
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
#include "EntityFactory.h"

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
    auto creatorSubSystem = SubSystemRegistry::getInstance().getSubSystem("DemoWorldCreator");
    auto creator = (DemoWorldCreator*) creatorSubSystem;
    auto eventLoopSubSystem = SubSystemRegistry::getInstance().getSubSystem("EventLoop");
    auto eventLoop = (EventLoop*) eventLoopSubSystem;
    return renderer->isReady() && creator->isReady() && eventLoop->isReady();
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
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto villager = factory->createEntity(EntityTypes::ET_VILLAGER);
    auto [transform, unit, selectible, playerComp] =
        gameState->getComponents<CompTransform, CompUnit, CompSelectible, CompPlayer>(villager);

    transform.position = Feet(tilePos.x * 256 + 128, tilePos.x * 256 + 50);
    transform.face(Direction::SOUTH);
    unit.commandQueue.push(ObjectPool<CmdIdle>::acquire(villager));
    playerComp.player = player;

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

    auto subSys = SubSystemRegistry::getInstance().getSubSystem("EventLoop");
    auto eventLoop = (EventLoop*) subSys;
    auto eventPublisher = (EventPublisher*) eventLoop;

    auto cmd = ObjectPool<CmdMove>::acquire();
    cmd->targetPos = target;
    Event event(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, unit});

    eventPublisher->publish(event);
}

int GameAPI::getCurrentAction(uint32_t unit)
{
    ScopedSynchronizer sync(m_sync);

    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto& action = gameState->getComponent<CompAction>(unit);
    return action.action;
}

Feet GameAPI::getUnitPosition(uint32_t unit)
{
    ScopedSynchronizer sync(m_sync);

    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto& transform = gameState->getComponent<CompTransform>(unit);
    return transform.position;
}

void GameAPI::deleteEntity(uint32_t entity)
{
    ScopedSynchronizer sync(m_sync);

    auto subSys = SubSystemRegistry::getInstance().getSubSystem("EventLoop");
    auto eventLoop = (EventLoop*) subSys;
    auto eventPublisher = (EventPublisher*) eventLoop;

    Event event(Event::Type::ENTITY_DELETE, EntityDeleteData{entity});

    eventPublisher->publish(event);
}
