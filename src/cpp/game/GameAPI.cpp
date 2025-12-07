#include "GameAPI.h"

#include "DemoWorldCreator.h"
#include "EntityFactory.h"
#include "EventLoop.h"
#include "EventPublisher.h"
#include "GameTypes.h"
#include "PlayerController.h"
#include "PlayerFactory.h"
#include "Renderer.h"
#include "ServiceRegistry.h"
#include "SubSystemRegistry.h"
#include "commands/CmdIdle.h"
#include "commands/CmdMove.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"

using namespace game;
using namespace core;

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
    auto renderer = static_pointer_cast<Renderer>(redndererSubSystem);
    auto creatorSubSystem = SubSystemRegistry::getInstance().getSubSystem("DemoWorldCreator");
    auto creator = static_pointer_cast<DemoWorldCreator>(creatorSubSystem);
    auto eventLoopSubSystem = SubSystemRegistry::getInstance().getSubSystem("EventLoop");
    auto eventLoop = static_pointer_cast<EventLoop>(eventLoopSubSystem);
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

    auto controller = ServiceRegistry::getInstance().getService<PlayerController>();
    return controller->getPlayer();
}

uint32_t GameAPI::createVillager(Ref<core::Player> player, const Feet& pos)
{
    ScopedSynchronizer sync(m_sync);

    auto tilePos = pos.toTile();
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto villager = factory->createEntity(EntityTypes::ET_VILLAGER, 0);
    auto [transform, unit, selectible, playerComp, vision] =
        stateMan->getComponents<CompTransform, CompUnit, CompSelectible, CompPlayer, CompVision>(
            villager);

    transform.position = Feet(tilePos.x * 256 + 128, tilePos.x * 256 + 50);
    transform.face(Direction::SOUTH);
    playerComp.player = player;

    auto newTile = transform.position.toTile();
    stateMan->gameMap().addEntity(MapLayerType::UNITS, newTile, villager);

    player->getFogOfWar()->markAsExplored(transform.position, vision.lineOfSight);

    return villager;
}

std::list<uint32_t> GameAPI::getVillagers()
{
    ScopedSynchronizer sync(m_sync);

    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    std::list<uint32_t> villagers;

    stateMan->getEntities<CompBuilder>().each([&villagers](uint32_t entity, CompBuilder&)
                                              { villagers.push_back(entity); });
    return villagers;
}

void GameAPI::commandToMove(uint32_t unit, const Feet& target)
{
    ScopedSynchronizer sync(m_sync);

    auto subSys = SubSystemRegistry::getInstance().getSubSystem("EventLoop");
    auto eventLoop = static_pointer_cast<EventLoop>(subSys);
    auto eventPublisher = static_pointer_cast<EventPublisher>(eventLoop);

    auto cmd = ObjectPool<CmdMove>::acquire();
    cmd->targetPos = target;
    Event event(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, unit});

    eventPublisher->publish(event);
}

int GameAPI::getCurrentAction(uint32_t unit)
{
    ScopedSynchronizer sync(m_sync);

    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto& action = stateMan->getComponent<CompAction>(unit);
    return action.action;
}

Feet GameAPI::getUnitPosition(uint32_t unit)
{
    ScopedSynchronizer sync(m_sync);

    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto& transform = stateMan->getComponent<CompTransform>(unit);
    return transform.position;
}

void GameAPI::deleteEntity(uint32_t entity)
{
    ScopedSynchronizer sync(m_sync);

    auto subSys = SubSystemRegistry::getInstance().getSubSystem("EventLoop");
    auto eventLoop = static_pointer_cast<EventLoop>(subSys);
    auto eventPublisher = static_pointer_cast<EventPublisher>(eventLoop);

    Event event(Event::Type::ENTITY_DELETE, EntityDeleteData{entity});

    eventPublisher->publish(event);
}
