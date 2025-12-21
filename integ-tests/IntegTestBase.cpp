#include "IntegTestBase.h"

#include <future>
#include "ServiceRegistry.h"
#include "TestWorldCreator.h"

using namespace core;
using namespace game;

game::Game IntegTestBase::m_game;
game::GameAPI* IntegTestBase::m_api = nullptr;
std::thread* IntegTestBase::m_testThread = nullptr;
core::Ref<game::IntegTestTickAssist> IntegTestBase::m_tickAssist;

void IntegTestBase::setup()
{
    m_tickAssist = core::CreateRef<game::IntegTestTickAssist>();
    auto sync = core::CreateRef<game::GameAPI::Synchronizer>();
    sync->onStart = [&](){m_tickAssist->aquireLock();};
    sync->onEnd = [&](){m_tickAssist->releaseLock();};

    m_api = new game::GameAPI(sync);
    m_testThread = new std::thread([]() {
            Game::Params params;
            params.populateWorld = false;
            params.revealAll = true;
            params.eventHandler = m_tickAssist;
            params.worldCreator = CreateRef<TestWorldCreator>();
            m_game.run(params);
    });

    // TODO: To undiscovered error, without this delay entt registry corrupts or 
    // goes into race condition in integ test setup. This is to prevent moving
    // ahead with tests before things settle down.
    sleep(2000);
 
    while (core::SubSystemRegistry::getInstance().isAllInitialized() == false) {}
    while (m_api->isReady() == false) {}

    m_api->executeCustomSynchronizedAction(
        []() { 
            auto settings = ServiceRegistry::getInstance().getService<Settings>();
            settings->setGameSpeed(2.0);
        });

}

void IntegTestBase::tearDown()
{
    m_api->quit();
    m_testThread->join();
    delete m_testThread;
    delete m_api;

    m_api = nullptr;
    m_testThread = nullptr;
}

void IntegTestBase::sleep(int ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void IntegTestBase::waitFor(std::function<bool()> condition, int timeoutMs, const std::string& msg)
{
    std::promise<bool> promisedFinished; 
    auto futureResult = promisedFinished.get_future();

    std::thread([&condition](std::promise<bool>& finished) 
    {
        while (true) 
        {
            if (condition()) 
            {
                finished.set_value(true);
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }, std::ref(promisedFinished)).detach();

    EXPECT_TRUE(futureResult.wait_for(std::chrono::milliseconds(timeoutMs)) != std::future_status::timeout)<< msg;
}
