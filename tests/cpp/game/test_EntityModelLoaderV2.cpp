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
class DummyDRSInterface : public DRSInterface
{
    core::Ref<drs::DRSFile> loadDRSFile(const std::string& filename)
    {
        return CreateRef<DRSFile>();
    }
    core::Rect<int> getBoundingBox(const std::string& drsFilename,
        int slpId,
        int frameIndex = 0)
    {
        return {};
    }
};


class EntityModelLoaderTest : public ::testing::Test
{
  public:
    void SetUp() override
    {
        m_typeReg = CreateRef<EntityTypeRegistry>();
        m_stateMan = CreateRef<StateManager>();
        m_drsInterface = CreateRef<DummyDRSInterface>();
        ServiceRegistry::getInstance().registerService(m_typeReg);
        ServiceRegistry::getInstance().registerService(m_stateMan);
    }

    void TearDown() override
    {
    }

    Ref<StateManager> m_stateMan;
    Ref<EntityTypeRegistry> m_typeReg;
    Ref<DRSInterface> m_drsInterface;
};
TEST_F(EntityModelLoaderTest, CreateEntitySimple)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("basic", m_drsInterface);
        loader.init();
        
        // Act
        EntityFactory* factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(3);

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

        EntityModelLoaderV2 loader("builder", m_drsInterface);
        loader.init();

        // Act
        EntityFactory* factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(3);

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
        EntityModelLoaderV2 loader("building", m_drsInterface);
        loader.init();

        // Act
        EntityFactory* factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(m_typeReg->getEntityType("mill"));

        // Assert
        auto& building = m_stateMan->getComponent<CompBuilding>(entity);
        EXPECT_EQ(building.size.value(), Size(2, 2));
        EXPECT_EQ(building.acceptedResourceNames.value().size(), 1);
        EXPECT_EQ(building.acceptedResourceNames.value()[0], "food");
        EXPECT_EQ(building.defaultOrientation.value(), BuildingOrientation::CORNER);
        EXPECT_EQ(building.connectedConstructionsAllowed.value(), false);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateResource)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("resource", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(m_typeReg->getEntityType("gold"));

        // Assert
        auto& resource = m_stateMan->getComponent<CompResource>(entity);
        EXPECT_EQ(resource.resourceName.value(), "gold");
        EXPECT_EQ(resource.resourceAmount.value(), 1000);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateGatherer)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("gatherer", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(m_typeReg->getEntityType("villager"));

        // Assert
        auto& gatherer = m_stateMan->getComponent<CompResourceGatherer>(entity);
        EXPECT_EQ(gatherer.capacity.value(), 100);
        EXPECT_EQ(gatherer.gatherSpeed.value(), 10);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateSelectables)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("selectable", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto villager = factory->createEntity(m_typeReg->getEntityType("villager"));
        auto mill = factory->createEntity(m_typeReg->getEntityType("mill"));
        auto gold = factory->createEntity(m_typeReg->getEntityType("gold"));

        // Assert
        auto& villagerSelectible = m_stateMan->getComponent<CompSelectible>(villager);
        auto& millSelectible = m_stateMan->getComponent<CompSelectible>(mill);
        auto& goldSelectible = m_stateMan->getComponent<CompSelectible>(gold);

        EXPECT_EQ(villagerSelectible.displayName.value(), "Villager");
        EXPECT_EQ(millSelectible.displayName.value(), "Mill");
        EXPECT_EQ(goldSelectible.displayName.value(), "Gold Mine");
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateTransform)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("transform", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto villager = factory->createEntity(m_typeReg->getEntityType("villager"));
        auto mill = factory->createEntity(m_typeReg->getEntityType("mill"));
        auto gold = factory->createEntity(m_typeReg->getEntityType("gold"));
        auto tiles = factory->createEntity(m_typeReg->getEntityType("default_tileset"));

        // Assert
        auto& villagerTransform = m_stateMan->getComponent<CompTransform>(villager);

        EXPECT_EQ(villagerTransform.speed.value(), 256);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateUnit)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("unit", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto militia = factory->createEntity(m_typeReg->getEntityType("militia"));

        // Assert
        auto& unitComp = m_stateMan->getComponent<CompUnit>(militia);

        EXPECT_EQ(unitComp.housingNeed.value(), 1);
        EXPECT_EQ(unitComp.type.value(), UnitType::INFANTRY);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateUnitFactory)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("unit_factory", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto barracks = factory->createEntity(m_typeReg->getEntityType("barracks"));

        // Assert
        auto& factoryComp = m_stateMan->getComponent<CompUnitFactory>(barracks);

        EXPECT_EQ(factoryComp.maxQueueSize.value(), 10);
        EXPECT_EQ(factoryComp.unitCreationSpeed.value(), 40);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateGarrison)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("garrison", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto tc = factory->createEntity(m_typeReg->getEntityType("town_center"));

        // Assert
        auto& garrison = m_stateMan->getComponent<CompGarrison>(tc);

        EXPECT_EQ(garrison.capacity.value(), 10);
        EXPECT_EQ(garrison.unitTypesInt.value().size(), 1);
        EXPECT_EQ(garrison.unitTypesInt.value()[0], 1);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateVision)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("vision", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto villager = factory->createEntity(m_typeReg->getEntityType("villager"));
        auto mill = factory->createEntity(m_typeReg->getEntityType("mill"));

        // Assert
        auto& villagerVision = m_stateMan->getComponent<CompVision>(villager);
        auto& millVision = m_stateMan->getComponent<CompVision>(mill);

        EXPECT_EQ(villagerVision.lineOfSight.value(), 256 * 5);
        EXPECT_EQ(villagerVision.lineOfSightShape.value(), LineOfSightShape::CIRCLE);
        EXPECT_EQ(villagerVision.activeTracking.value(), false);

        EXPECT_EQ(millVision.lineOfSight.value(), 256 * 6);
        EXPECT_EQ(millVision.lineOfSightShape.value(), LineOfSightShape::ROUNDED_SQUARE);
        EXPECT_EQ(millVision.activeTracking.value(), true);

    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, CreateHousing)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("housing", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto house = factory->createEntity(m_typeReg->getEntityType("house"));

        // Assert
        auto& housingComp = m_stateMan->getComponent<CompHousing>(house);

        EXPECT_EQ(housingComp.housingCapacity.value(), 5);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, DefaultEmptyComponents)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader("transform", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto villager = factory->createEntity(m_typeReg->getEntityType("villager"));
        auto mill = factory->createEntity(m_typeReg->getEntityType("mill"));
        auto gold = factory->createEntity(m_typeReg->getEntityType("gold"));
        auto tiles = factory->createEntity(m_typeReg->getEntityType("default_tileset"));

        // Assert
        EXPECT_TRUE(m_stateMan->hasComponent<CompAnimation>(villager));

        EXPECT_TRUE(m_stateMan->hasComponent<CompAction>(villager));

        EXPECT_TRUE(m_stateMan->hasComponent<CompRendering>(villager));
        EXPECT_TRUE(m_stateMan->hasComponent<CompRendering>(mill));
        EXPECT_TRUE(m_stateMan->hasComponent<CompRendering>(gold));
        EXPECT_TRUE(m_stateMan->hasComponent<CompRendering>(tiles));

        EXPECT_TRUE(m_stateMan->hasComponent<CompPlayer>(villager));
        EXPECT_TRUE(m_stateMan->hasComponent<CompPlayer>(mill));

        EXPECT_TRUE(m_stateMan->hasComponent<CompGraphics>(villager));
        EXPECT_TRUE(m_stateMan->hasComponent<CompGraphics>(mill));
        EXPECT_TRUE(m_stateMan->hasComponent<CompGraphics>(gold));
        EXPECT_TRUE(m_stateMan->hasComponent<CompGraphics>(tiles));

        EXPECT_TRUE(m_stateMan->hasComponent<CompTransform>(mill));
        EXPECT_TRUE(m_stateMan->hasComponent<CompTransform>(gold));
        EXPECT_TRUE(m_stateMan->hasComponent<CompTransform>(tiles));
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}
} // namespace game