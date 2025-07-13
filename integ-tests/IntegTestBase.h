#ifndef INTEGTESTBASE_H
#define INTEGTESTBASE_H

#include "Event.h"
#include "Game.h"
#include "GameAPI.h"
#include "IntegTestTickAssist.h"

#include <gtest/gtest.h>
#include <chrono>

class IntegTestBase : public ::testing::Test 
{
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    void sleep(int ms);

    static game::Game m_game;
    static game::GameAPI* m_api;
    static std::thread* m_testThread;
    static ion::Ref<game::IntegTestTickAssist> m_tickAssist;
};

#endif