#include <gtest/gtest.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include "EntityDefinitionLoader.h"
#include "components/CompAnimation.h"
#include "GameTypes.h"

using namespace std;
using namespace ion;
using namespace game;
namespace py = pybind11;

py::dict createAnimationEntry(std::string name, int frameCount, int speed, bool repeatable) {
        py::dict anim;
        anim["name"] = name;
        anim["frame_count"] = frameCount;
        anim["speed"] = speed;
        anim["repeatable"] = repeatable;
        return anim;
    }

// Test createAnimation returns monostate if no animations
TEST(EntityDefinitionLoaderTest, CreateAnimationReturnsMonostateIfNoAnimations) {
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;
    auto comp = EntityDefinitionLoader::createAnimation(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createBuilder returns monostate if no build_speed
TEST(EntityDefinitionLoaderTest, CreateBuilderReturnsMonostateIfNoBuildSpeed) {
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityDefinitionLoader::createBuilder(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompResourceGatherer returns monostate if no gather_speed
TEST(EntityDefinitionLoaderTest, CreateCompResourceGathererReturnsMonostateIfNoGatherSpeed) {
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityDefinitionLoader::createCompResourceGatherer(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompUnit returns monostate if no line_of_sight
TEST(EntityDefinitionLoaderTest, CreateCompUnitReturnsMonostateIfNotAUnit) {
    py::scoped_interpreter guard{};
    py::dict d;
    
    py::exec(R"()");
    py::object module = py::module_::import("__main__");

    auto comp = EntityDefinitionLoader::createCompUnit(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompTransform returns monostate if no moving_speed
TEST(EntityDefinitionLoaderTest, CreateCompTransformReturnsMonostateIfNoMovingSpeed) {
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityDefinitionLoader::createCompTransform(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompResource returns monostate if no resource_amount
TEST(EntityDefinitionLoaderTest, CreateCompResourceReturnsMonostateIfNoResourceAmount) {
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
    EXPECT_EQ(walk.frames, 10);
    EXPECT_EQ(walk.speed, 100);
    EXPECT_TRUE(walk.repeatable);

    auto idle = comp.animations[UnitAction::IDLE];
    EXPECT_EQ(idle.frames, 5);
    EXPECT_EQ(idle.speed, 50);
    EXPECT_FALSE(idle.repeatable);
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
        self.size = 'small'

entity = House()
    )");

    py::object houseDef = py::globals()["entity"];
    py::object module = py::module_::import("__main__");

    ComponentType result = EntityDefinitionLoader::createCompBuilding(module, houseDef);
    ASSERT_TRUE(std::holds_alternative<CompBuilding>(result));
    EXPECT_EQ(std::get<CompBuilding>(result).lineOfSight, 5);
    EXPECT_EQ(std::get<CompBuilding>(result).size.width, 2);
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

    ComponentType result = EntityDefinitionLoader::createCompBuilding(module, notBuilding);
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

class EntityDefinitionLoaderExposure : public EntityDefinitionLoader
{
  public:
      void loadBuildings(pybind11::object module)
      {
          EntityDefinitionLoader::loadBuildings(module);
      }
};


TEST(EntityDefinitionLoaderTest, LoadAllBuilding)
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
        size='small',
        accepted_resources=['food'], 
        graphics={'default':Graphic(slp_id=3483)}
    )]
    )");

    py::object module = py::module_::import("__main__");

    EntityDefinitionLoaderExposure loader;
    loader.loadBuildings(module);

    GraphicsID id;
    id.entityType = EntityTypes::ET_MILL;
    auto drsData = loader.getDRSData(id);
    EXPECT_EQ(drsData.slpId, 3483);
}