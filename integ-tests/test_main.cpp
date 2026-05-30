#include <gtest/gtest.h>
#include "IntegTestBase.h"

#include "EntityModelLoaderV2.h"


class GlobalEnv : public ::testing::Environment
{
  public:
    void SetUp() override
    {
        game::EntityModelLoaderV2::g_sysPaths.push_back(PYTHON_MODEL_DIR);

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