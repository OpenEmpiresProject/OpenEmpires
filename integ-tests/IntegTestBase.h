#ifndef INTEGTESTBASE_H
#define INTEGTESTBASE_H

#include "Event.h"
#include "Game.h"
#include "GameAPI.h"
#include "IntegTestTickAssist.h"

#include <gtest/gtest.h>
#include <chrono>

#define STRINGIFY_IMPL(x) #x
#define ASSERT_WAIT_FOR(condition, timeout) IntegTestBase::waitFor([&](){return condition;}, timeout, "Condition: " STRINGIFY_IMPL(condition))

class IntegTestBase : public ::testing::Test 
{
public:
    static void SetUpTestSuite();
    static void TearDownTestSuite();

    static void sleep(int ms);
    static void waitFor(std::function<bool()> condition, int timeoutMs, const std::string& msg);

    static game::Game m_game;
    static game::GameAPI* m_api;
    static std::thread* m_testThread;
    static core::Ref<game::IntegTestTickAssist> m_tickAssist;
};

#endif