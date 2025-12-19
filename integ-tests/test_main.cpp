#include <gtest/gtest.h>
#include "IntegTestBase.h"

class GlobalEnv : public ::testing::Environment
{
  public:
    void SetUp() override
    {
        IntegTestBase::setup();
    }

    void TearDown() override
    {
        IntegTestBase::tearDown();
    }
};

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    ::testing::AddGlobalTestEnvironment(new GlobalEnv);
    return RUN_ALL_TESTS();
}