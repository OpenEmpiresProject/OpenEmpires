#include <gtest/gtest.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include "EntityDefinitionLoader.h"
#include "components/CompAnimation.h"
#include "GameTypes.h"

#include  <filesystem>

using namespace std;
using namespace ion;
using namespace drs;
namespace py = pybind11;

namespace game
{

class EntityDefinitionLoaderExposure : public EntityDefinitionLoader
{
  public:
    void loadBuildings(pybind11::object module)
    {
        EntityDefinitionLoader::loadBuildings(module);
    }

    void loadConstructionSites(pybind11::object module)
    {
        EntityDefinitionLoader::loadConstructionSites(module);
    }

    void loadTileSets(pybind11::object module)
    {
        EntityDefinitionLoader::loadTileSets(module);
    }

    void loadNaturalResources(pybind11::object module)
    {
        EntityDefinitionLoader::loadNaturalResources(module);
    }

    void loadUIElements(pybind11::object module)
    {
        EntityDefinitionLoader::loadUIElements(module);
    }

    EntityDefinitionLoader::ConstructionSiteData getSite(const std::string& sizeStr)
    {
        return EntityDefinitionLoader::getSite(sizeStr);
    }

    void setSite(const std::string& sizeStr, const std::map<int, int>& progressToFrame)
    {
        EntityDefinitionLoader::setSite(sizeStr, progressToFrame);
    }

    uint32_t createEntity(uint32_t entityType) override
    {
        return EntityDefinitionLoader::createEntity(entityType);
    }

    void setDRSData(int64_t id, const EntityDRSData& data)
    {
        EntityDefinitionLoader::setDRSData(id, data);
    }

    void setDRSLoaderFunc(std::function<Ref<DRSFile>(const std::string&)> func)
    {
        EntityDefinitionLoader::setDRSLoaderFunc(func);
    }
};

py::dict createAnimationEntry(std::string name, int frameCount, int speed, bool repeatable)
{
    py::dict anim;
    anim["name"] = name;
    anim["frame_count"] = frameCount;
    anim["speed"] = speed;
    anim["repeatable"] = repeatable;
    return anim;
}

// Test createAnimation returns monostate if no animations
TEST(EntityDefinitionLoaderTest, CreateAnimationReturnsMonostateIfNoAnimations)
{
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;
    auto comp = EntityDefinitionLoader::createAnimation(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createBuilder returns monostate if no build_speed
TEST(EntityDefinitionLoaderTest, CreateBuilderReturnsMonostateIfNoBuildSpeed)
{
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityDefinitionLoader::createBuilder(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompResourceGatherer returns monostate if no gather_speed
TEST(EntityDefinitionLoaderTest, CreateCompResourceGathererReturnsMonostateIfNoGatherSpeed)
{
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityDefinitionLoader::createCompResourceGatherer(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompUnit returns monostate if no line_of_sight
TEST(EntityDefinitionLoaderTest, CreateCompUnitReturnsMonostateIfNotAUnit)
{
    py::scoped_interpreter guard{};
    py::dict d;

    py::exec(R"()");
    py::object module = py::module_::import("__main__");

    auto comp = EntityDefinitionLoader::createCompUnit(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompTransform returns monostate if no moving_speed
TEST(EntityDefinitionLoaderTest, CreateCompTransformReturnsMonostateIfNoMovingSpeed)
{
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityDefinitionLoader::createCompTransform(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompResource returns monostate if no resource_amount
TEST(EntityDefinitionLoaderTest, CreateCompResourceReturnsMonostateIfNoResourceAmount)
{
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityDefinitionLoader::createCompResource(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

TEST(EntityDefinitionLoaderTest, ParsesAnimationsCorrectly)
{
    py::scoped_interpreter guard{};

    py::exec(R"(
class Animation:
    def __init__(self, name, frame_count, speed, repeatable):
        self.name = name
        self.frame_count = frame_count
        self.speed = speed
        self.repeatable = repeatable

class DummyEntity:
    def __init__(self):
        self.animations = [
            Animation("move", 10, 100, True),
            Animation("idle", 5, 50, False),
        ]

entity = DummyEntity()
)");

    py::object entityDef = py::globals()["entity"];
    py::object module = py::module_::import("__main__");

    ComponentType result = EntityDefinitionLoader::createAnimation(module, entityDef);
    ASSERT_TRUE(std::holds_alternative<CompAnimation>(result));

    auto comp = std::get<CompAnimation>(result);

    auto walk = comp.animations[UnitAction::MOVE];
    EXPECT_EQ(walk.value().frames, 10);
    EXPECT_EQ(walk.value().speed, 100);
    EXPECT_TRUE(walk.value().repeatable);

    auto idle = comp.animations[UnitAction::IDLE];
    EXPECT_EQ(idle.value().frames, 5);
    EXPECT_EQ(idle.value().speed, 50);
    EXPECT_FALSE(idle.value().repeatable);
}

TEST(EntityDefinitionLoaderTest, ReturnsComponentForBuildingSubclass)
{
    py::scoped_interpreter guard{};

    py::exec(R"(
class Building:
    def __init__(self):
        self.line_of_sight = 5

class House(Building):
    def __init__(self):
        super().__init__()
        self.size = 'medium'

entity = House()
    )");

    py::object houseDef = py::globals()["entity"];
    py::object module = py::module_::import("__main__");

    EntityDefinitionLoaderExposure loader;
    loader.setSite("medium", std::map<int, int>());
    ComponentType result = loader.createCompBuilding(module, houseDef);
    ASSERT_TRUE(std::holds_alternative<CompBuilding>(result));
    EXPECT_EQ(std::get<CompBuilding>(result).lineOfSight, 5);
    EXPECT_EQ(std::get<CompBuilding>(result).size.value().width, 2);
}

TEST(EntityDefinitionLoaderTest, SkipsBuildingComponentForUnrelatedClass)
{
    py::scoped_interpreter guard{};

    py::exec(R"(
class NotABuilding:
    def __init__(self):
        self.line_of_sight = 10
    )");

    py::object notBuilding = py::eval("NotABuilding()");
    py::object module = py::module_::import("__main__");

    EntityDefinitionLoaderExposure loader;
    ComponentType result = loader.createCompBuilding(module, notBuilding);
    ASSERT_TRUE(std::holds_alternative<std::monostate>(result));
}

TEST(EntityDefinitionLoaderTest, ReturnsComponentForUnitSubclass)
{
    py::scoped_interpreter guard{};

    py::exec(R"(
class Unit:
    name = ''
    line_of_sight = 100
    moving_speed = 256
    animations = []

class Villager(Unit):
    def __init__(self):
        super().__init__()
        self.name = 'villager'
        self.build_speed = 10

entity = Villager()
    )");

    py::object def = py::globals()["entity"];
    py::object module = py::module_::import("__main__");

    ComponentType result = EntityDefinitionLoader::createCompUnit(module, def);
    ASSERT_TRUE(std::holds_alternative<CompUnit>(result));
    EXPECT_EQ(std::get<CompUnit>(result).lineOfSight, 100);
}

TEST(EntityDefinitionLoaderTest, LoadAllBuilding)
{
    // Arrange
    py::scoped_interpreter guard{};

    py::exec(R"(
class Graphic:
    drs_file = "graphics.drs"
    slp_id = 0
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Building:
    name = ''
    line_of_sight = 0
    size = ''
    graphics = []

class ResourceDropOff:
    accepted_resources = []

class SingleResourceDropOffPoint(Building, ResourceDropOff):
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

all_buildings= [
    SingleResourceDropOffPoint(
        name='mill', 
        line_of_sight=256,
        size='medium',
        accepted_resources=['food'], 
        graphics={'default':Graphic(slp_id=3483)}
    )]
    )");

    py::object module = py::module_::import("__main__");

    auto cwd = std::filesystem::current_path();

    EntityDefinitionLoaderExposure loader;
    loader.setSite("medium", std::map<int, int>());
    GraphicsID siteId;
    siteId.entityType = EntityTypes::ET_CONSTRUCTION_SITE;
    siteId.entitySubType = 2;
    loader.setDRSData(siteId.hash(), EntityDefinitionLoader::EntityDRSData{.slpId = 111});
    loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });

    // Act
    loader.loadBuildings(module);

    // Assert
    // Main building
    EXPECT_EQ(loader.getDRSData(GraphicsID{.entityType = EntityTypes::ET_MILL}).slpId, 3483);
    // One of construction sites
    EXPECT_EQ(
        loader.getDRSData(GraphicsID{.entityType = EntityTypes::ET_MILL, .entitySubType = 2}).slpId,
        111);
}

TEST(EntityDefinitionLoaderTest, BuildingResourceAcceptance)
{
    // Arrange
    py::scoped_interpreter guard{};

    py::exec(R"(
class Graphic:
    drs_file = "graphics.drs"
    slp_id = 0
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Building:
    name = ''
    line_of_sight = 0
    size = ''
    graphics = []

class ResourceDropOff:
    accepted_resources = []

class SingleResourceDropOffPoint(Building, ResourceDropOff):
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

mill = SingleResourceDropOffPoint(
        name='mill', 
        line_of_sight=256,
        size='medium',
        accepted_resources=['food'], 
        graphics={'default':Graphic(slp_id=3483)}
    )
    )");

    py::object millDef = py::globals()["mill"];
    py::object module = py::module_::import("__main__");

    EntityDefinitionLoaderExposure loader;
    loader.setSite("medium", std::map<int, int>());
    ComponentType result = loader.createCompBuilding(module, millDef);

    ASSERT_TRUE(std::get<CompBuilding>(result).canDropOff(ResourceType::FOOD));
}

TEST(EntityDefinitionLoaderTest, LoadAllConstructionSites)
{
    py::scoped_interpreter guard{};

    py::exec(R"(
class Graphic:
    drs_file = "graphics.drs"
    slp_id = 0
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class ConstructionSite:
    name = 'construction_site'
    size = ''
    graphics = {}
    progress_frame_map = {}
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


all_construction_sites= [
    ConstructionSite(
        size='small',
        progress_frame_map={33:1, 66:2, 99:3}, 
        graphics={'default':Graphic(slp_id=236)}
    ),
    ConstructionSite(
        size='medium',
        progress_frame_map={33:1, 66:2, 99:3}, 
        graphics={'default':Graphic(slp_id=237)}
    )]
    )");

    py::object module = py::module_::import("__main__");

    EntityDefinitionLoaderExposure loader;
    loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });
    loader.loadConstructionSites(module);

    {
        GraphicsID id;
        id.entityType = EntityTypes::ET_CONSTRUCTION_SITE;
        id.entitySubType = EntitySubTypes::EST_SMALL_SIZE;
        auto drsData = loader.getDRSData(id);
        EXPECT_EQ(drsData.slpId, 236);

        auto sideData = loader.getSite("small");
        EXPECT_EQ(sideData.size.width, 1);
        EXPECT_EQ(sideData.size.height, 1);
        EXPECT_EQ(sideData.progressToFrames.at(33), 1);
    }

    {
        GraphicsID id;
        id.entityType = EntityTypes::ET_CONSTRUCTION_SITE;
        id.entitySubType = EntitySubTypes::EST_MEDIUM_SIZE;
        auto drsData = loader.getDRSData(id);
        EXPECT_EQ(drsData.slpId, 237);

        auto sideData = loader.getSite("medium");
        EXPECT_EQ(sideData.size.width, 2);
        EXPECT_EQ(sideData.size.height, 2);
        EXPECT_EQ(sideData.progressToFrames.at(66), 2);
    }
}

TEST(EntityDefinitionLoaderTest, LoadBuildingsWithConstructionSiteLinks)
{
    py::scoped_interpreter guard{};

    py::exec(R"(
class Graphic:
    drs_file = "graphics.drs"
    slp_id = 0
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Building:
    name = ''
    line_of_sight = 0
    size = ''
    graphics = []

class ResourceDropOff:
    accepted_resources = []

class SingleResourceDropOffPoint(Building, ResourceDropOff):
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

all_buildings= [
    SingleResourceDropOffPoint(
        name='mill', 
        line_of_sight=256,
        size='medium',
        accepted_resources=['food'], 
        graphics={'default':Graphic(slp_id=3483)}
    )]

class ConstructionSite:
    name = 'construction_site'
    size = ''
    graphics = {}
    progress_frame_map = {}
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


all_construction_sites= [
    ConstructionSite(
        size='medium',
        progress_frame_map={33:1, 66:2, 99:3}, 
        graphics={'default':Graphic(slp_id=237)}
    )]
    )");

    py::object module = py::module_::import("__main__");

    EntityDefinitionLoaderExposure loader;
    loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });
    loader.loadConstructionSites(module);
    loader.loadBuildings(module);

    GraphicsID id;
    id.entityType = EntityTypes::ET_MILL;
    auto drsData = loader.getDRSData(id);
    EXPECT_EQ(drsData.slpId, 3483);

    auto gameState = std::make_shared<ion::GameState>();
    ion::ServiceRegistry::getInstance().registerService(gameState);

    auto mill = loader.createEntity(EntityTypes::ET_MILL);

    auto building = gameState->getComponent<CompBuilding>(mill);
    EXPECT_EQ(building.visualVariationByProgress.at(66), 2);
}

TEST(EntityDefinitionLoaderTest, LoadTileSets)
{
    // Arrange
    py::scoped_interpreter guard{};

    py::exec(R"(
class Graphic:
    drs_file = ""
    slp_id = 0
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

class TileSet:
    graphics = {}
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

all_tilesets = [
    TileSet(graphics={"grass":Graphic(drs_file="terrain.drs", slp_id=15001)})
]
    )");

    py::object module = py::module_::import("__main__");

    EntityDefinitionLoaderExposure loader;
    loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });

    // Act
    loader.loadTileSets(module);

    // Assert
    EXPECT_EQ(loader.getDRSData(GraphicsID{.entityType = EntityTypes::ET_TILE}).slpId, 15001);
}

TEST(EntityDefinitionLoaderTest, LoadTreeWithStump)
{
    // Arrange
    py::scoped_interpreter guard{};

    py::exec(R"(
class Graphic:
    drs_file = "graphics.drs"
    slp_id = 0
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

class NaturalResource:
    name = ''
    resource_amount = 100
    graphics = {}
    def __init__(self, **kwargs): self.__dict__.update(kwargs)


class Tree(NaturalResource):
    shadow_graphics = {}
    stump = None
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

all_natural_resources= [
    Tree(
        name="wood",
        resource_amount=100,
        graphics={"oak":Graphic(slp_id=435)},
        stump=NaturalResource(name="stump", graphics={"oak":Graphic(slp_id=1252)})
    )]
    )");

    py::object module = py::module_::import("__main__");

    // Act
    EntityDefinitionLoaderExposure loader;
    loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });
    loader.loadNaturalResources(module);

    // Assert
    EXPECT_EQ(loader.getDRSData(GraphicsID{.entityType = EntityTypes::ET_TREE}).slpId, 435);
    EXPECT_EQ(loader
                  .getDRSData(GraphicsID{.entityType = EntityTypes::ET_TREE,
                                         .entitySubType = EntitySubTypes::EST_CHOPPED_TREE})
                  .slpId,
              1252);
}

TEST(EntityDefinitionLoaderTest, LoadUIElements)
{
    py::scoped_interpreter guard{};

    py::exec(R"(
class Rect:
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

class Graphic:
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

class UIElement:
    def __init__(self, **kwargs): self.__dict__.update(kwargs)

all_ui_elements = [
    UIElement(name="resource_panel", graphics={"default":Graphic(drs_file="interfac.drs", slp_id=51101, clip_rect=Rect(w=400, h=25))})
]
    )");

    py::object module = py::module_::import("__main__");

    // Act
    EntityDefinitionLoaderExposure loader;
    loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });
    loader.loadUIElements(module);

    // Assert
    GraphicsID id;
    id.entityType = EntityTypes::ET_UI_ELEMENT;
    id.action = 1;
    EXPECT_EQ(loader.getDRSData(id).slpId, 51101);
    EXPECT_EQ(loader.getDRSData(id).clipRect.w,
              400);
    EXPECT_EQ(loader.getDRSData(id).clipRect.h,
              25);
}
} // namespace game