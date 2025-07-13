#include "IntegTestBase.h"

#include <future>

game::Game IntegTestBase::m_game;
game::GameAPI* IntegTestBase::m_api = nullptr;
std::thread* IntegTestBase::m_testThread = nullptr;
ion::Ref<game::IntegTestTickAssist> IntegTestBase::m_tickAssist;

void IntegTestBase::SetUpTestSuite() 
{
    m_tickAssist = ion::CreateRef<game::IntegTestTickAssist>();
    auto sync = ion::CreateRef<game::GameAPI::Synchronizer>();
    sync->onStart = [&](){m_tickAssist->aquireLock();};
    sync->onEnd = [&](){m_tickAssist->releaseLock();};

    m_api = new game::GameAPI(sync);
    m_testThread = new std::thread([]() {
        m_game.runIntegTestEnv(m_tickAssist);
    });
    while (ion::SubSystemRegistry::getInstance().isAllInitialized() == false) {}
    m_api->isReady();
}

void IntegTestBase::TearDownTestSuite() 
{
    m_api->quit();
    m_testThread->join();
    delete m_testThread;
    delete m_api;
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
