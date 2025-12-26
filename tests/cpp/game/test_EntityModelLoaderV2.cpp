#include <gtest/gtest.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include "EntityModelLoaderV2.h"
#include "components/CompAnimation.h"
#include "GameTypes.h"

#include  <filesystem>
#include "EntityTypeRegistry.h"
#include "EntityFactory.h"

using namespace std;
using namespace core;
using namespace drs;
namespace py = pybind11;

namespace game
{
class EntityModelLoaderTest : public ::testing::Test
{
  public:
    void SetUp() override
    {
        m_typeReg = CreateRef<EntityTypeRegistry>();
        m_stateMan = CreateRef<StateManager>();
        ServiceRegistry::getInstance().registerService(m_typeReg);
        ServiceRegistry::getInstance().registerService(m_stateMan);
    }

    void TearDown() override
    {
    }

    Ref<StateManager> m_stateMan;
    Ref<EntityTypeRegistry> m_typeReg;
};
TEST_F(EntityModelLoaderTest, CreateEntitySimple)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("basic");
        loader.init();
        
        // Act
        EntityFactory* factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(3, 0);

        // Assert
        auto& info = m_stateMan->getComponent<CompEntityInfo>(entity);
        EXPECT_EQ(info.entityName.value(), "villager");
        EXPECT_EQ(info.entityType.value(), 3);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateBuilder)
{
    try
    {
        // Arrange
        m_typeReg->registerEntityType("mill", 10);
        m_typeReg->registerEntityType("wood_camp", 11);
        m_typeReg->registerEntityType("mine_camp", 12);

        EntityModelLoaderV2 loader("builder");
        loader.init();

        // Act
        EntityFactory* factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(3, 0);

        // Assert
        auto& builder = m_stateMan->getComponent<CompBuilder>(entity);
        EXPECT_EQ(builder.buildSpeed.value(), 40);
        EXPECT_EQ(builder.buildableTypesByShortcut.value().size(),3);
        EXPECT_EQ(builder.buildableNameByShortcut.value().size(), 3);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}


TEST_F(EntityModelLoaderTest, CreateBuilding)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("building");
        loader.init();

        // Act
        EntityFactory* factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(m_typeReg->getEntityType("mill"), 0);

        // Assert
        auto& building = m_stateMan->getComponent<CompBuilding>(entity);
        EXPECT_EQ(building.sizeName.value(), "medium");
        EXPECT_EQ(building.acceptedResourceNames.value().size(), 1);
        EXPECT_EQ(building.acceptedResourceNames.value()[0], "food");
        EXPECT_EQ(building.defaultOrientationName.value(), "corner");
        EXPECT_EQ(building.connectedConstructionsAllowed.value(), false);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

} // namespace game