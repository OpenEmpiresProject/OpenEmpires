#include <gtest/gtest.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include "EntityModelLoaderV2.h"
#include "components/CompAnimation.h"
#include "GameTypes.h"
#include "utils/Types.h"

#include  <filesystem>
#include "EntityTypeRegistry.h"
#include "EntityFactory.h"
#include "commands/CmdIdle.h"

using namespace std;
using namespace core;
using namespace drs;
namespace py = pybind11;

namespace game
{
class DummyDRSInterface : public DRSInterface
{
  public:
    Rect<int> boundingBoxToReturn;

  private:
    core::Ref<drs::DRSFile> loadDRSFile(const std::string& filename)
    {
        return CreateRef<DRSFile>();
    }
    core::Rect<int> getBoundingBox(const std::string& drsFilename,
        int slpId,
        int frameIndex = 0)
    {
        return boundingBoxToReturn;
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
        //py::finalize_interpreter();
    }

    Ref<StateManager> m_stateMan;
    Ref<EntityTypeRegistry> m_typeReg;
    Ref<DRSInterface> m_drsInterface;
    const std::string m_baseScriptDir = std::string(TEST_PYTHON_MODEL_DIR) + "/";
};
TEST_F(EntityModelLoaderTest, CreateEntitySimple)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader(m_baseScriptDir, "basic.model_importer", m_drsInterface);
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

        EntityModelLoaderV2 loader(m_baseScriptDir, "builder.model_importer", m_drsInterface);
        loader.init();

        // Act
        EntityFactory* factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(3);

        // Assert
        auto& builder = m_stateMan->getComponent<CompBuilder>(entity);
        EXPECT_EQ(builder.buildSpeed.value(), 40);
        EXPECT_EQ(builder.buildableTypesByShortcut.value().size(), 3);
        EXPECT_EQ(builder.buildableTypesByShortcut.value().at('m'), m_typeReg->getEntityType("mill"));
        EXPECT_EQ(builder.buildableTypesByShortcut.value().at('l'),
                  m_typeReg->getEntityType("wood_camp"));
        EXPECT_EQ(builder.buildableTypesByShortcut.value().at('n'),
                  m_typeReg->getEntityType("mine_camp"));
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
        EntityModelLoaderV2 loader(m_baseScriptDir, "building.model_importer", m_drsInterface);
        loader.init();

        // Act
        EntityFactory* factory = dynamic_cast<EntityFactory*>(&loader);
        const auto entityType = m_typeReg->getEntityType("mill");
        auto entity = factory->createEntity(entityType);

        // Assert
        auto& building = m_stateMan->getComponent<CompBuilding>(entity);
        EXPECT_EQ(building.size.value(), Size(2, 2));
        EXPECT_EQ(building.acceptedResourceNames.value().size(), 1);
        EXPECT_EQ(building.acceptedResourceNames.value()[0], "food");
        EXPECT_EQ(building.defaultOrientation.value(), BuildingOrientation::CORNER);
        EXPECT_EQ(building.connectedConstructionsAllowed.value(), false);
        EXPECT_EQ(building.dropOffForResourceType.value(), ResourceType::FOOD);

        // Validate building graphics
        auto drsProvider = dynamic_cast<GraphicsLoadupDataProvider*>(&loader);
        GraphicsID gid(entityType);
        auto& data = (DRSData&)drsProvider->getData(gid);
        EXPECT_EQ(data.parts.size(), 1);
        EXPECT_EQ(data.parts[0].slpId, 3483);

        // Validate icon graphics
        gid.isIcon = true;
        auto& iconData = (DRSData&) drsProvider->getData(gid);
        EXPECT_EQ(iconData.parts.size(), 1);
        EXPECT_EQ(iconData.parts[0].slpId, 50705);
        EXPECT_EQ(iconData.parts[0].frameIndex, 21);

        // Validate construction site graphics
        gid.isConstructing = 1;
        gid.isIcon = 0;
        auto& constructData = (DRSData&) drsProvider->getData(gid);
        EXPECT_EQ(constructData.parts.size(), 1);
        EXPECT_EQ(constructData.parts[0].slpId, 237);

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
        Ref<DummyDRSInterface> dummyDRS =
            std::dynamic_pointer_cast<DummyDRSInterface>(m_drsInterface);
        dummyDRS->boundingBoxToReturn = Rect<int>(0, 0, 32, 32);

        EntityModelLoaderV2 loader(m_baseScriptDir, "resource.model_importer", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto entity = factory->createEntity(m_typeReg->getEntityType("gold"));

        // Assert
        auto& resource = m_stateMan->getComponent<CompResource>(entity);
        auto& selectible = m_stateMan->getComponent<CompSelectible>(entity);

        EXPECT_EQ(resource.resourceName.value(), "gold");
        EXPECT_EQ(resource.resourceAmount.value(), 1000);
        EXPECT_EQ(resource.remainingAmount, 1000); // Not a property, but should be initialized
        EXPECT_EQ(resource.original.value().amount, 1000);
        EXPECT_EQ(resource.original.value().type, ResourceType::GOLD);

        auto boundingBoxes = selectible.boundingBoxes.value();
        EXPECT_EQ(boundingBoxes.at(0, static_cast<int>(Direction::SOUTH)), Rect<int>(0, 0, 32, 32));
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
        EntityModelLoaderV2 loader(m_baseScriptDir, "gatherer.model_importer", m_drsInterface);
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
        EntityModelLoaderV2 loader(m_baseScriptDir, "selectable.model_importer", m_drsInterface);
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
        EntityModelLoaderV2 loader(m_baseScriptDir, "transform.model_importer", m_drsInterface);
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
        Ref<DummyDRSInterface> dummyDRS = std::dynamic_pointer_cast<DummyDRSInterface>(m_drsInterface);
        dummyDRS->boundingBoxToReturn = Rect<int>(0, 0, 32, 32);

        EntityModelLoaderV2 loader(m_baseScriptDir, "unit.model_importer", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto militia = factory->createEntity(m_typeReg->getEntityType("militia"));

        // Assert
        auto& unitComp = m_stateMan->getComponent<CompUnit>(militia);
        auto& graphicComp = m_stateMan->getComponent<CompGraphics>(militia);
        auto& selectible = m_stateMan->getComponent<CompSelectible>(militia);
        auto& transform = m_stateMan->getComponent<CompTransform>(militia);
        auto& attack = m_stateMan->getComponent<CompAttack>(militia);
        auto& health = m_stateMan->getComponent<CompHealth>(militia);
        auto& armor = m_stateMan->getComponent<CompArmor>(militia);

        EXPECT_EQ(unitComp.housingNeed.value(), 1);
        EXPECT_EQ(unitComp.type.value(), UnitType::INFANTRY);

        // Verify default command is CmdIdle and entity is set correctly
        auto defaultCmd = std::dynamic_pointer_cast<core::CmdIdle>(unitComp.defaultCommand.value());
        ASSERT_NE(defaultCmd, nullptr);
        EXPECT_EQ(defaultCmd->getEntityID(), (uint32_t)entt::null);

        Command* firstCmd = unitComp.commandQueue.top();
        ASSERT_NE(dynamic_cast<core::CmdIdle*>(firstCmd), nullptr);
        EXPECT_EQ(firstCmd->getEntityID(), militia);

        EXPECT_EQ(graphicComp.layer.value(), GraphicLayer::ENTITIES);
        EXPECT_EQ(selectible.displayName.value(), "Militia");

        auto boundingBoxes = selectible.boundingBoxes.value();
        EXPECT_EQ(boundingBoxes.at(0, static_cast<int>(Direction::SOUTH)),
                  Rect<int>(0, 0, 32, 32));

        EXPECT_EQ(transform.hasRotation.value(), true);

        EXPECT_EQ(attack.attackPerClass[(int) ArmorClass::INFANTRY], 0);
        EXPECT_EQ(attack.attackPerClass[(int) ArmorClass::MELEE], 10);
        EXPECT_EQ(attack.attackPerClass[(int) ArmorClass::PIERCE], 5);
        EXPECT_FLOAT_EQ(attack.attackMultiplierPerClass[(int) ArmorClass::INFANTRY], 1);
        EXPECT_FLOAT_EQ(attack.attackMultiplierPerClass[(int) ArmorClass::MELEE], 1.5);
        EXPECT_FLOAT_EQ(attack.attackMultiplierPerClass[(int) ArmorClass::PIERCE], 0.5);

        EXPECT_EQ(armor.armorPerClass[(int) ArmorClass::INFANTRY], 1000); // Base armor
        EXPECT_EQ(armor.armorPerClass[(int) ArmorClass::MELEE], 20);
        EXPECT_EQ(armor.armorPerClass[(int) ArmorClass::PIERCE], 25);
        EXPECT_EQ(armor.baseArmor, 1000);
        EXPECT_FLOAT_EQ(armor.damageResistance.value(), 0.1);

        EXPECT_EQ(health.maxHealth.value(), 100);
        EXPECT_FLOAT_EQ(health.health, 100);
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
        Ref<DummyDRSInterface> dummyDRS =
            std::dynamic_pointer_cast<DummyDRSInterface>(m_drsInterface);
        dummyDRS->boundingBoxToReturn = Rect<int>(0, 0, 32, 32);

        EntityModelLoaderV2 loader(m_baseScriptDir, "unit_factory.model_importer", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto barracks = factory->createEntity(m_typeReg->getEntityType("barracks"));

        // Assert
        auto& factoryComp = m_stateMan->getComponent<CompUnitFactory>(barracks);
        auto& selectible = m_stateMan->getComponent<CompSelectible>(barracks);
        auto& transform = m_stateMan->getComponent<CompTransform>(barracks);

        EXPECT_EQ(factoryComp.maxQueueSize.value(), 10);
        EXPECT_EQ(factoryComp.unitCreationSpeed.value(), 40);
        EXPECT_EQ(selectible.displayName.value(), "Barracks");

        auto boundingBoxes = selectible.boundingBoxes.value();
        EXPECT_EQ(boundingBoxes.at(0, static_cast<int>(Direction::NONE)), Rect<int>(0, 0, 32, 32));

        EXPECT_EQ(factoryComp.producibleUnitShortcuts.value().size(), 1);
        EXPECT_EQ(factoryComp.producibleUnitShortcuts.value().at('m'),
                  m_typeReg->getEntityType("militia"));

        EXPECT_EQ(transform.hasRotation.value(), false);

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
        EntityModelLoaderV2 loader(m_baseScriptDir, "garrison.model_importer", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto tc = factory->createEntity(m_typeReg->getEntityType("town_center"));

        // Assert
        auto& garrison = m_stateMan->getComponent<CompGarrison>(tc);

        EXPECT_EQ(garrison.capacity.value(), 10);
        EXPECT_EQ(garrison.unitTypes.value().size(), 1);
        EXPECT_TRUE(garrison.unitTypes.value().contains(UnitType::VILLAGER));
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
        EntityModelLoaderV2 loader(m_baseScriptDir, "vision.model_importer", m_drsInterface);
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
        EntityModelLoaderV2 loader(m_baseScriptDir, "housing.model_importer", m_drsInterface);
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
        EntityModelLoaderV2 loader(m_baseScriptDir, "transform.model_importer", m_drsInterface);
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

TEST_F(EntityModelLoaderTest, BuildingOrientation)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader(m_baseScriptDir, "graphic-orientation.model_importer", m_drsInterface);
        loader.init();

        // Act
        EntityFactory* factory = dynamic_cast<EntityFactory*>(&loader);
        const auto entityType = m_typeReg->getEntityType("palisade");
        auto entity = factory->createEntity(entityType);

        // Assert
        auto& building = m_stateMan->getComponent<CompBuilding>(entity);
        EXPECT_EQ(building.size.value(), Size(1, 1));
        EXPECT_EQ(building.defaultOrientation.value(), BuildingOrientation::CORNER);
        EXPECT_EQ(building.connectedConstructionsAllowed.value(), true);

        // Validate building graphics
        auto drsProvider = dynamic_cast<GraphicsLoadupDataProvider*>(&loader);
        GraphicsID gid(entityType);
        {
            gid.orientation = static_cast<uint64_t>(BuildingOrientation::DIAGONAL_FORWARD);
            auto& data = (DRSData&) drsProvider->getData(gid);
            EXPECT_EQ(data.parts.size(), 1);
            EXPECT_EQ(data.parts[0].slpId, 1828);
            EXPECT_EQ(data.parts[0].frameIndex, 0);
        }
        {
            gid.orientation = static_cast<uint64_t>(BuildingOrientation::DIAGONAL_BACKWARD);
            auto& data = (DRSData&) drsProvider->getData(gid);
            EXPECT_EQ(data.parts.size(), 1);
            EXPECT_EQ(data.parts[0].slpId, 1828);
            EXPECT_EQ(data.parts[0].frameIndex, 1);
        }
        {
            gid.orientation = static_cast<uint64_t>(BuildingOrientation::CORNER);
            auto& data = (DRSData&) drsProvider->getData(gid);
            EXPECT_EQ(data.parts.size(), 1);
            EXPECT_EQ(data.parts[0].slpId, 1828);
            EXPECT_EQ(data.parts[0].frameIndex, 2);
        }
        {

            gid.orientation = static_cast<uint64_t>(BuildingOrientation::HORIZONTAL);
            auto& data = (DRSData&) drsProvider->getData(gid);
            EXPECT_EQ(data.parts.size(), 1);
            EXPECT_EQ(data.parts[0].slpId, 1828);
            EXPECT_EQ(data.parts[0].frameIndex, 3);
        }
        {

            gid.orientation = static_cast<uint64_t>(BuildingOrientation::VERTICAL);
            auto& data = (DRSData&) drsProvider->getData(gid);
            EXPECT_EQ(data.parts.size(), 1);
            EXPECT_EQ(data.parts[0].slpId, 1828);
            EXPECT_EQ(data.parts[0].frameIndex, 4);
        }
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST_F(EntityModelLoaderTest, Animations)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader(m_baseScriptDir, "unit.model_importer", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        const auto entityType = m_typeReg->getEntityType("militia");
        auto militia = factory->createEntity(entityType);

        // Assert
        // Validate DRS data
        auto drsProvider = dynamic_cast<GraphicsLoadupDataProvider*>(&loader);
        GraphicsID gid(entityType);
        {
            gid.action = UnitAction::IDLE;
            auto& data = (DRSData&) drsProvider->getData(gid);
            EXPECT_EQ(data.parts.size(), 1);
            EXPECT_EQ(data.parts[0].slpId, 993);
        }
        {
            gid.action = UnitAction::MOVE;
            auto& data = (DRSData&) drsProvider->getData(gid);
            EXPECT_EQ(data.parts.size(), 1);
            EXPECT_EQ(data.parts[0].slpId, 997);
        }

        // Validate CompAnimation
        EXPECT_TRUE(m_stateMan->hasComponent<CompAnimation>(militia));
        auto& compAnimation = m_stateMan->getComponent<CompAnimation>(militia);
        EXPECT_EQ(compAnimation.animations[UnitAction::IDLE].value().frames, 6);
        EXPECT_EQ(compAnimation.animations[UnitAction::MOVE].value().frames, 12);
        EXPECT_EQ(compAnimation.animations[UnitAction::IDLE].value().speed, 15);
        EXPECT_EQ(compAnimation.animations[UnitAction::MOVE].value().speed, 10); // Default value

    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}


TEST_F(EntityModelLoaderTest, CreateUIElement)
{
    try
    {
        // Arrange
        EntityModelLoaderV2 loader(m_baseScriptDir, "ui.model_importer", m_drsInterface);
        loader.init();

        // Act
        auto factory = dynamic_cast<EntityFactory*>(&loader);
        auto ui = factory->createEntity(m_typeReg->getEntityType("ui_element"));

        // Assert
        auto& uiComp = m_stateMan->getComponent<CompUIElement>(ui);
        auto& info = m_stateMan->getComponent<CompEntityInfo>(ui);

        EXPECT_EQ(uiComp.uiElementType.value(), (int) UIElementTypes::RESOURCE_PANEL);
        EXPECT_EQ(info.entityType.value(), (int) EntityTypes::ET_UI_ELEMENT);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}


} // namespace game