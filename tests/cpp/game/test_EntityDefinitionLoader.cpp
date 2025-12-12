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

	static std::string strip_utf8_bom(std::string s)
    {
        static const std::string bom = "\xEF\xBB\xBF";
        if (s.rfind(bom, 0) == 0)
        { // starts with bom
            s.erase(0, bom.size());
        }
        return s;
    }

	static std::string readCoreDefs()
	{
        std::string coreDefsScript;
        // Load the `scripts/core_defs.py` file content into `coreDefsScript`.
        // Try a few relative locations based on common test working directories.
        try
        {
            namespace fs = std::filesystem;
            std::vector<fs::path> candidates = {
                fs::current_path() / "assets" / "scripts" / "core_defs.py",
                fs::current_path() / ".." / "assets" / "scripts" / "core_defs.py",
                fs::current_path() / ".." / ".." / "assets" / "scripts" / "core_defs.py",
                fs::current_path() / ".." / ".." / ".." / "assets" / "scripts" / "core_defs.py",
            };

            for (const auto& p : candidates)
            {
                if (fs::exists(p) && fs::is_regular_file(p))
                {
                    std::ifstream ifs(p, std::ios::in | std::ios::binary);
                    if (ifs)
                    {
                        coreDefsScript.assign((std::istreambuf_iterator<char>(ifs)),
                                              std::istreambuf_iterator<char>());
                    }
                    break;
                }
            }
        }
        catch (...)
        {
            // Best-effort loading for tests; leave coreDefsScript empty on failure.
            coreDefsScript.clear();
        }
        return strip_utf8_bom(coreDefsScript);
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
	    std::string code = EntityDefinitionLoaderExposure::readCoreDefs() + R"(
all_entity_names = [
    "mill"
]

all_buildings= [
	ResourceDropOff(
	    name='mill', 
	    line_of_sight=256,
        line_of_sight_shape="circle",
        active_tracking=True,
	    size='medium',
	    accepted_resources=['food'], 
	    graphics=Graphic(by_theme={"default":SingleGraphic(slp_id=3483)}),
	)]
	        )";


        py::exec(code);
	
	    py::object module = py::module_::import("__main__");
	
	    auto cwd = std::filesystem::current_path();
	
	    EntityDefinitionLoaderExposure loader;
	    loader.setSite("medium", std::map<int, int>());
	    GraphicsID siteId;
	    siteId.entityType = EntityTypes::ET_CONSTRUCTION_SITE;
	    siteId.entitySubType = 2;
	    EntityLoader::EntityDRSData data;
	    data.parts.push_back(EntityLoader::EntityDRSData::Part(nullptr, 111, Vec2::null, std::nullopt));
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
        std::string code = EntityDefinitionLoaderExposure::readCoreDefs() + 
        R"(
mill = ResourceDropOff(
		name='mill', 
		line_of_sight=256,
		size='medium',
		accepted_resources=['food'], 
		graphics={'default':Graphic(slp_id=3483)}
	)
		        )";

        py::exec(code);

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
		std::string code = EntityDefinitionLoaderExposure::readCoreDefs() +
        R"(
all_entity_names = [
    "construction_site"
]

all_construction_sites= [
	ConstructionSite(
	    size='small',
	    progress_frame_map={33:1, 66:2, 99:3}, 
	    graphics=Graphic(by_theme={"default":SingleGraphic(slp_id=236)}),
	),
	ConstructionSite(
	    size='medium',
	    progress_frame_map={33:1, 66:2, 99:3}, 
	    graphics=Graphic(by_theme={"default":SingleGraphic(slp_id=237)}),
	)]
	        )";
        py::exec(code);

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
        std::string code = EntityDefinitionLoaderExposure::readCoreDefs() +
	    R"(
all_entity_names = [
    "construction_site",
    "mill"
]

all_buildings= [
	ResourceDropOff(
	    name='mill', 
	    line_of_sight=256,
        line_of_sight_shape="circle",
        active_tracking=True,
	    size='medium',
	    accepted_resources=['food'], 
	    graphics=Graphic(by_theme={"default":SingleGraphic(slp_id=3483)}),
	)]
	
all_construction_sites= [
	ConstructionSite(
	    size='medium',
	    progress_frame_map={33:1, 66:2, 99:3}, 
	    graphics=Graphic(by_theme={"default":SingleGraphic(slp_id=237)}),
	)]
	        )";

        py::exec(code);
	
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
        std::string code = EntityDefinitionLoaderExposure::readCoreDefs() + R"(
all_tilesets = [
	TileSet(
        name="default_tileset",
        graphics=Graphic(by_theme={"grass":SingleGraphic(drs_file="terrain.drs", slp_id=15001)})
    )
]
	        )";

        py::exec(code);
	
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
	    std::string code = EntityDefinitionLoaderExposure::readCoreDefs() +	R"(
all_entity_names = [
    "wood"
]

all_natural_resources= [
	Tree(
        name="wood",
        display_name="Tree",
        resource_amount=100,
        graphics=Graphic(by_theme={"default":SingleGraphic(slp_id=435)}),
        stump=NaturalResourceAdditionalPart(
            name="stump", 
            graphics=Graphic(by_theme={"oak":SingleGraphic(slp_id=1252)})),
        shadow=NaturalResourceAdditionalPart(
            name="shadow", 
            graphics=Graphic(by_theme={"oak":SingleGraphic(slp_id=2296)})),
        icon=Icon(drs_file="interfac.drs", slp_id=50731, index=0),
    )]
			)";

        py::exec(code);
	
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
        std::string code = EntityDefinitionLoaderExposure::readCoreDefs() + 
		R"(
all_ui_elements = [
	 UIElement(name="resource_panel", 
              graphics=Graphic(by_theme={"default":SingleGraphic(drs_file="interfac.drs", slp_id=51101, clip_rect=Rect(x=0, y=0, w=400, h=25))})),
]
			)";

        py::exec(code);
	
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