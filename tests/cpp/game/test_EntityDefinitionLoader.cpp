#include <gtest/gtest.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include "EntityLoader.h"
#include "components/CompAnimation.h"
#include "GameTypes.h"

#include  <filesystem>
#include "EntityTypeRegistry.h"

using namespace std;
using namespace core;
using namespace drs;
namespace py = pybind11;

namespace game
{

class EntityDefinitionLoaderExposure : public EntityLoader
{
  public:
	EntityDefinitionLoaderExposure()
	{
        auto registry = CreateRef<EntityTypeRegistry>();
        ServiceRegistry::getInstance().registerService(registry);
	}

	void loadEntityTypes(pybind11::object module)
	{
        EntityLoader::loadEntityTypes(module);
	}

    void loadBuildings(pybind11::object module)
    {
        EntityLoader::loadBuildings(module);
    }

    void loadConstructionSites(pybind11::object module)
    {
        EntityLoader::loadConstructionSites(module);
    }

    void loadTileSets(pybind11::object module)
    {
        EntityLoader::loadTileSets(module);
    }

    void loadNaturalResources(pybind11::object module)
    {
        EntityLoader::loadNaturalResources(module);
    }

    void loadUIElements(pybind11::object module)
    {
        EntityLoader::loadUIElements(module);
    }

    EntityLoader::ConstructionSiteData getSite(const std::string& sizeStr)
    {
        return EntityLoader::getSite(sizeStr);
    }

    void setSite(const std::string& sizeStr, const std::map<int, int>& progressToFrame)
    {
        EntityLoader::setSite(sizeStr, progressToFrame);
    }

    uint32_t createEntity(uint32_t entityType, uint32_t entitySubType) override
    {
        return EntityLoader::createEntity(entityType, entitySubType);
    }

    void setDRSData(const GraphicsID& id, const EntityDRSData& data)
    {
        EntityLoader::setDRSData(id, data);
    }

    void setDRSLoaderFunc(std::function<Ref<DRSFile>(const std::string&)> func)
    {
        EntityLoader::setDRSLoaderFunc(func);
    }

    void setBoundingBoxReadFunc(
        std::function<core::Rect<int>(core::Ref<drs::DRSFile>, uint32_t)> func)
    {
        EntityLoader::setBoundingBoxReadFunc(func);
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
    auto comp = EntityLoader::createAnimation(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createBuilder returns monostate if no build_speed
TEST(EntityDefinitionLoaderTest, CreateBuilderReturnsMonostateIfNoBuildSpeed)
{
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityLoader::createBuilder(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompResourceGatherer returns monostate if no gather_speed
TEST(EntityDefinitionLoaderTest, CreateCompResourceGathererReturnsMonostateIfNoGatherSpeed)
{
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityLoader::createCompResourceGatherer(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompUnit returns monostate if no line_of_sight
TEST(EntityDefinitionLoaderTest, CreateCompUnitReturnsMonostateIfNotAUnit)
{
    py::scoped_interpreter guard{};
    py::dict d;

    py::exec(R"()");
    py::object module = py::module_::import("__main__");

    auto comp = EntityLoader::createCompUnit(1, module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompTransform returns monostate if no moving_speed
TEST(EntityDefinitionLoaderTest, CreateCompTransformReturnsMonostateIfNoMovingSpeed)
{
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityLoader::createCompTransform(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

// Test createCompResource returns monostate if no resource_amount
TEST(EntityDefinitionLoaderTest, CreateCompResourceReturnsMonostateIfNoResourceAmount)
{
    py::scoped_interpreter guard{};
    py::dict d;
    py::object module;

    auto comp = EntityLoader::createCompResource(module, d);
    EXPECT_TRUE(std::holds_alternative<std::monostate>(comp));
}

TEST(EntityDefinitionLoaderTest, ParsesAnimationsCorrectly)
{
	// Lock has to be acquired before the try block to allow catch block to 
	// read the error (since that also requires a lock)
    py::scoped_interpreter guard{};
  
	try
    {
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

        ComponentType result = EntityLoader::createAnimation(module, entityDef);
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
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST(EntityDefinitionLoaderTest, ReturnsComponentForBuildingSubclass)
{
    py::scoped_interpreter guard{};
    try
    {
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
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST(EntityDefinitionLoaderTest, SkipsBuildingComponentForUnrelatedClass)
{
    py::scoped_interpreter guard{};
   
	try
    {
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
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST(EntityDefinitionLoaderTest, ReturnsComponentForUnitSubclass)
{
    py::scoped_interpreter guard{};
    try
    {
        py::exec(R"(
class Unit:
	name = ''
	line_of_sight = 100
	moving_speed = 256
	animations = []
	housing_need = 1
	
class Villager(Unit):
	def __init__(self):
		super().__init__()
		self.name = 'villager'
		self.build_speed = 10
		self.unit_type = 1

entity = Villager()
	        )");

        py::object def = py::globals()["entity"];
        py::object module = py::module_::import("__main__");
        EntityDefinitionLoaderExposure loader;
        ComponentType result = EntityLoader::createCompUnit(1, module, def);
        ASSERT_TRUE(std::holds_alternative<CompUnit>(result));
        EXPECT_EQ(std::get<CompUnit>(result).lineOfSight, 100);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST(EntityDefinitionLoaderTest, LoadAllBuilding)
{
    py::scoped_interpreter guard{};

    try
    {
        // Arrange	
	    std::string code = R"(
all_entity_names = [
    "mill"
]

class Graphic:
	drs_file = "graphics.drs"
	slp_id = 0
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
class CompositeGraphic:
	pass
	
class Building:
	name = ''
	line_of_sight = 0
	size = ''
	graphics = []
	
class ResourceDropOff(Building):
	accepted_resources = []
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
all_buildings= [
	ResourceDropOff(
	    name='mill', 
	    line_of_sight=256,
	    size='medium',
	    accepted_resources=['food'], 
	    graphics={'default':[Graphic(slp_id=3483)]}
	)]
	        )";


  //      py::object builtins = py::module_::import("builtins");
  //      py::object compile = builtins.attr("compile");

		//// First: check syntax
  //      compile(code, "<embedded>", "exec");

        py::exec(code);
	
	    py::object module = py::module_::import("__main__");
	
	    auto cwd = std::filesystem::current_path();
	
	    EntityDefinitionLoaderExposure loader;
	    loader.setSite("medium", std::map<int, int>());
	    GraphicsID siteId;
	    siteId.entityType = EntityTypes::ET_CONSTRUCTION_SITE;
	    siteId.entitySubType = 2;
	    EntityLoader::EntityDRSData data;
	    data.parts.push_back(EntityLoader::EntityDRSData::Part(nullptr, 111, Vec2::null));
	    loader.setDRSData(siteId, data);
	    loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });
	    loader.setBoundingBoxReadFunc([](core::Ref<drs::DRSFile>, uint32_t) -> core::Rect<int>
	                                    { return core::Rect<int>(); });
        loader.loadEntityTypes(module);
	
	    // Act
	    loader.loadBuildings(module);
	
	    // Assert
        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
        auto millType = typeRegistry->getEntityType("mill");
	    // Main building
        EXPECT_EQ(loader.getDRSData(GraphicsID(millType)).parts[0].slpId, 3483);
	    // One of construction sites
        EXPECT_EQ(loader.getDRSData(GraphicsID(millType, 2)).parts[0].slpId, 111);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST(EntityDefinitionLoaderTest, BuildingResourceAcceptance)
{
    py::scoped_interpreter guard{};
    try
    {
        py::exec(R"(
class Graphic:
	drs_file = "graphics.drs"
	slp_id = 0
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
		
class CompositeGraphic:
	pass
		
class Building:
	name = ''
	line_of_sight = 0
	size = ''
	graphics = []
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
		
class ResourceDropOff(Building):
	accepted_resources = []
		
mill = ResourceDropOff(
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
        ASSERT_TRUE(std::get<CompBuilding>(result).acceptResource(ResourceType::FOOD));
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}
 
TEST(EntityDefinitionLoaderTest, LoadAllConstructionSites)
{
    py::scoped_interpreter guard{};
   
	try
    {
        py::exec(R"(
all_entity_names = [
    "construction_site"
]

class Graphic:
	drs_file = "graphics.drs"
	slp_id = 0
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
class CompositeGraphic:
	pass
	
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
	    graphics={'default':[Graphic(slp_id=236)]}
	),
	ConstructionSite(
	    size='medium',
	    progress_frame_map={33:1, 66:2, 99:3}, 
	    graphics={'default':[Graphic(slp_id=237)]}
	)]
	        )");

        py::object module = py::module_::import("__main__");

        EntityDefinitionLoaderExposure loader;
        loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile>
                                { return nullptr; });
        loader.setBoundingBoxReadFunc([](core::Ref<drs::DRSFile>, uint32_t) -> core::Rect<int>
                                      { return core::Rect<int>(); });
        loader.loadEntityTypes(module);
        loader.loadConstructionSites(module);

        {
            GraphicsID id;
            id.entityType = EntityTypes::ET_CONSTRUCTION_SITE;
            id.entitySubType = EntitySubTypes::EST_SMALL_SIZE;
            auto drsData = loader.getDRSData(id);
            EXPECT_EQ(drsData.parts[0].slpId, 236);

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
            EXPECT_EQ(drsData.parts[0].slpId, 237);

            auto sideData = loader.getSite("medium");
            EXPECT_EQ(sideData.size.width, 2);
            EXPECT_EQ(sideData.size.height, 2);
            EXPECT_EQ(sideData.progressToFrames.at(66), 2);
        }
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST(EntityDefinitionLoaderTest, LoadBuildingsWithConstructionSiteLinks)
{
    py::scoped_interpreter guard{};
  
	try
    {
	    py::exec(R"(
all_entity_names = [
    "construction_site",
    "mill"
]

class Graphic:
	drs_file = "graphics.drs"
	slp_id = 0
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
class CompositeGraphic:
	pass
	
class Building:
	name = ''
	line_of_sight = 0
	size = ''
	graphics = []
	
class ResourceDropOff(Building):
	accepted_resources = []
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
all_buildings= [
	ResourceDropOff(
	    name='mill', 
	    line_of_sight=256,
	    size='medium',
	    accepted_resources=['food'], 
	    graphics={'default':[Graphic(slp_id=3483)]}
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
	    graphics={'default':[Graphic(slp_id=237)]}
	)]
	        )");
	
	    py::object module = py::module_::import("__main__");
	
	    EntityDefinitionLoaderExposure loader;
	    loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });
	    loader.setBoundingBoxReadFunc([](core::Ref<drs::DRSFile>, uint32_t) -> core::Rect<int>
	                                    { return core::Rect<int>(); });
        loader.loadEntityTypes(module);
	    loader.loadConstructionSites(module);
	    loader.loadBuildings(module);
	
		auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
        auto millType = typeRegistry->getEntityType("mill");

	    GraphicsID id;
        id.entityType = millType;
	    auto drsData = loader.getDRSData(id);
	    EXPECT_EQ(drsData.parts[0].slpId, 3483);
	
	    auto stateMan = std::make_shared<core::StateManager>();
	    core::ServiceRegistry::getInstance().registerService(stateMan);
	
	    auto mill = loader.createEntity(millType, 0);
	
	    auto building = stateMan->getComponent<CompBuilding>(mill);
	    EXPECT_EQ(building.visualVariationByProgress.at(66), 2);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}

TEST(EntityDefinitionLoaderTest, LoadTileSets)
{
    py::scoped_interpreter guard{};
 
	try
    {
        py::exec(R"(
class Graphic:
	drs_file = ""
	slp_id = 0
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
class CompositeGraphic:
	pass
	
class TileSet:
	graphics = {}
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
all_tilesets = [
	TileSet(graphics={"grass":[Graphic(drs_file="terrain.drs", slp_id=15001)]})
]
	        )");
	
	    py::object module = py::module_::import("__main__");
	
	    EntityDefinitionLoaderExposure loader;
	    loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });
	    loader.setBoundingBoxReadFunc([](core::Ref<drs::DRSFile>, uint32_t) -> core::Rect<int>
	                                    { return core::Rect<int>(); });
	
	    // Act
	    loader.loadTileSets(module);
	
	    // Assert
	    EXPECT_EQ(loader.getDRSData(GraphicsID(EntityTypes::ET_TILE)).parts[0].slpId, 15001);
    }
    catch (const py::error_already_set& e)
    {
        FAIL() << e.what();
    }
}
TEST(EntityDefinitionLoaderTest, LoadTreeWithStump)
{
    py::scoped_interpreter guard{};
   
	try
	{
		py::exec(R"(
all_entity_names = [
    "wood"
]

class Graphic:
	drs_file = "graphics.drs"
	slp_id = 0
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
class CompositeGraphic:
	pass
	
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
		graphics={"oak":[Graphic(slp_id=435)]},
		stump=NaturalResource(name="stump", graphics={"oak":[Graphic(slp_id=1252)]})
	)]
			)");
	
		py::object module = py::module_::import("__main__");
	
		// Act
		EntityDefinitionLoaderExposure loader;
		loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });
		loader.setBoundingBoxReadFunc([](core::Ref<drs::DRSFile>, uint32_t) -> core::Rect<int>
										{ return core::Rect<int>(); });
        loader.loadEntityTypes(module);
		loader.loadNaturalResources(module);
	
		// Assert
		EXPECT_EQ(loader.getDRSData(GraphicsID(EntityTypes::ET_TREE)).parts[0].slpId, 435);
		EXPECT_EQ(loader
						.getDRSData(GraphicsID(EntityTypes::ET_TREE, EntitySubTypes::EST_CHOPPED_TREE))
						.parts[0]
						.slpId,
					1252);
	}
	catch (const py::error_already_set& e)
	{
		FAIL() << e.what();
	}
}

TEST(EntityDefinitionLoaderTest, LoadUIElements)
{
    py::scoped_interpreter guard{};
   
	try
	{
		py::exec(R"(
class Rect:
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
class Graphic:
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
class CompositeGraphic:
	pass
	
class UIElement:
	def __init__(self, **kwargs): self.__dict__.update(kwargs)
	
all_ui_elements = [
	UIElement(name="resource_panel", graphics={"default":[Graphic(drs_file="interfac.drs", slp_id=51101, clip_rect=Rect(w=400, h=25))]})
]
			)");
	
		py::object module = py::module_::import("__main__");
	
		// Act
		EntityDefinitionLoaderExposure loader;
		loader.setDRSLoaderFunc([](const std::string& drsFilename) -> Ref<DRSFile> { return nullptr; });
		loader.setBoundingBoxReadFunc([](core::Ref<drs::DRSFile>, uint32_t) -> core::Rect<int>
										{ return core::Rect<int>(); });
		loader.loadUIElements(module);
	
		// Assert
		GraphicsID id;
		id.entityType = EntityTypes::ET_UI_ELEMENT;
		id.entitySubType = EntitySubTypes::EST_UI_RESOURCE_PANEL;
		EXPECT_EQ(loader.getDRSData(id).parts[0].slpId, 51101);
		EXPECT_EQ(loader.getDRSData(id).clipRect.w,
					400);
		EXPECT_EQ(loader.getDRSData(id).clipRect.h,
					25);
	}
	catch (const py::error_already_set& e)
	{
		FAIL() << e.what();
	}
}
} // namespace game