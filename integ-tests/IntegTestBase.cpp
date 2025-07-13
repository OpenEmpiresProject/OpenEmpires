#include "IntegTestBase.h"

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