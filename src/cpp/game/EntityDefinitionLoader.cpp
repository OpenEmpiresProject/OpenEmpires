#include "EntityDefinitionLoader.h"

#include "GameTypes.h"
#include "commands/CmdIdle.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

#include <unordered_map>

using namespace ion;
using namespace game;
using namespace std;
using namespace drs;
namespace py = pybind11;

UnitAction getAction(const std::string actionname)
{
    static unordered_map<string, UnitAction> actions = {
        {"idle", UnitAction::IDLE},
        {"move", UnitAction::MOVE},
        {"chop", UnitAction::CHOPPING},
        {"mine", UnitAction::MINING},
        {"carry_lumber", UnitAction::CARRYING_LUMBER},
        {"carry_gold", UnitAction::CARRYING_GOLD},
        {"carry_stone", UnitAction::CARRYING_STONE},
        {"build", UnitAction::BUILDING},
    };

    return actions.at(actionname);
}

uint32_t getEntityType(const std::string& entityName)
{
    static unordered_map<string, uint32_t> entityTypes = {
        {"villager", EntityTypes::ET_VILLAGER},
        {"wood", EntityTypes::ET_TREE},
        {"gold", EntityTypes::ET_GOLD},
        {"stone", EntityTypes::ET_STONE},
        {"mill", EntityTypes::ET_MILL},
        {"wood_camp", EntityTypes::ET_LUMBER_CAMP},
        {"mine_camp", EntityTypes::ET_MINING_CAMP},
    };
    return entityTypes.at(entityName);
}

ResourceType getResourceType(const std::string& name)
{
    static unordered_map<string, ResourceType> resTypes = {
        {"wood", ResourceType::WOOD},
        {"gold", ResourceType::GOLD},
        {"stone", ResourceType::STONE},
    };
    return resTypes.at(name);
}

Size getBuildingSize(const std::string& name)
{
    if (name == "small")
        return Size(2, 2);
    else if (name == "medium")
        return Size(3, 3);
    else
    {
        debug_assert(false, "unknown building size");
        return Size(0, 0);
    }
}

ComponentType EntityDefinitionLoader::createAnimation(py::object module,
                                                      pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "animations") == false)
        return std::monostate{};

    CompAnimation comp;
    for (auto py_anim : entityDefinition.attr("animations"))
    {
        CompAnimation::ActionAnimation animation;
        animation.frames = EntityDefinitionLoader::readValue<int>(py_anim, "frame_count");
        animation.speed = EntityDefinitionLoader::readValue<int>(py_anim, "speed");
        animation.repeatable = EntityDefinitionLoader::readValue<bool>(py_anim, "repeatable");
        auto name = EntityDefinitionLoader::readValue<string>(py_anim, "name");
        auto action = getAction(name);

        comp.animations[action] = animation;
    }
    return ComponentType(comp);
}

ComponentType EntityDefinitionLoader::createBuilder(py::object module,
                                                    pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "build_speed") == false)
        return std::monostate{};

    auto buildSpeed = entityDefinition.attr("build_speed").cast<int>();
    return ComponentType(CompBuilder(buildSpeed));
}

ComponentType EntityDefinitionLoader::createCompResourceGatherer(py::object module,
                                                                 pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "gather_speed") == false)
        return std::monostate{};

    CompResourceGatherer comp;
    comp.gatherSpeed = entityDefinition.attr("gather_speed").cast<int>();
    comp.capacity = entityDefinition.attr("resource_capacity").cast<int>();
    return ComponentType(comp);
}

ComponentType EntityDefinitionLoader::createCompUnit(py::object module,
                                                     pybind11::handle entityDefinition)
{
    if (py::hasattr(module, "Unit"))
    {
        py::object unitClass = module.attr("Unit");
        if (py::isinstance(entityDefinition, unitClass))
        {
            CompUnit comp;
            comp.lineOfSight = entityDefinition.attr("line_of_sight").cast<int>();
            return ComponentType(comp);
        }
    }
    return std::monostate{};
}

ComponentType EntityDefinitionLoader::createCompTransform(py::object module,
                                                          pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "moving_speed") == false)
        return std::monostate{};

    CompTransform comp;
    comp.speed = entityDefinition.attr("moving_speed").cast<int>();
    comp.hasRotation = true;
    return ComponentType(comp);
}

ComponentType EntityDefinitionLoader::createCompResource(py::object module,
                                                         pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "resource_amount") == false)
        return std::monostate{};

    auto name = EntityDefinitionLoader::readValue<string>(entityDefinition, "name");
    auto amount = EntityDefinitionLoader::readValue<int>(entityDefinition, "resource_amount");

    return ComponentType(CompResource(Resource(getResourceType(name), amount)));
}

ComponentType EntityDefinitionLoader::createCompBuilding(py::object module,
                                                         pybind11::handle entityDefinition)
{
    if (py::hasattr(module, "Building"))
    {
        py::object buildingClass = module.attr("Building");
        if (py::isinstance(entityDefinition, buildingClass))
        {
            CompBuilding comp;
            comp.lineOfSight = readValue<int>(entityDefinition, "line_of_sight");
            comp.size = getBuildingSize(readValue<std::string>(entityDefinition, "size"));

            return ComponentType(comp);
        }
    }
    return std::monostate{};
}

EntityDefinitionLoader::EntityDefinitionLoader(/* args */)
{
}

EntityDefinitionLoader::~EntityDefinitionLoader()
{
}

void EntityDefinitionLoader::load()
{
    py::scoped_interpreter guard{};

    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("insert")(0, "./assets/scripts");

    py::object module = py::module_::import("defs");
    loadUnits(module);
    loadNaturalResources(module);
}

void EntityDefinitionLoader::loadUnits(py::object module)
{
    if (py::hasattr(module, "all_units"))
    {
        py::list all_units = module.attr("all_units");

        for (auto py_unit : all_units)
        {
            std::string name = py_unit.attr("name").cast<std::string>();

            auto entityType = getEntityType(name);
            addComponentsForUnit(entityType);
            createOrUpdateComponent(module, entityType, py_unit);
            updateDRSData(entityType, py_unit);
        }
    }
}

void EntityDefinitionLoader::loadNaturalResources(py::object module)
{
    if (py::hasattr(module, "all_natural_resources"))
    {
        py::list entries = module.attr("all_natural_resources");

        for (auto entry : entries)
        {
            std::string name = entry.attr("name").cast<std::string>();

            auto entityType = getEntityType(name);
            createOrUpdateComponent(module, entityType, entry);
            updateDRSData(entityType, entry);
        }
    }
}

void EntityDefinitionLoader::loadBuildings(pybind11::object module)
{
    if (py::hasattr(module, "all_buildings"))
    {
        py::list entries = module.attr("all_buildings");

        for (auto entry : entries)
        {
            std::string name = entry.attr("name").cast<std::string>();

            auto entityType = getEntityType(name);
            createOrUpdateComponent(module, entityType, entry);
            updateDRSData(entityType, entry);
        }
    }
}

EntityDefinitionLoader::EntityDRSData EntityDefinitionLoader::getDRSData(const GraphicsID& id)
{
    auto it = m_DRSDataByGraphicsIdHash.find(id.hash());
    if (it != m_DRSDataByGraphicsIdHash.end())
        return it->second;
    else
        spdlog::warn("Could not find DRS data for id {}", id.toString());
    return EntityDRSData();
}

uint32_t EntityDefinitionLoader::createEntity(uint32_t entityType)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto entity = gameState->createEntity();

    auto it = m_componentsByEntityType.find(entityType);
    if (it != m_componentsByEntityType.end())
    {
        for (const auto& variantComponent : it->second)
        {
            std::visit(
                [&](auto&& comp)
                {
                    using T = std::decay_t<decltype(comp)>;
                    if constexpr (!std::is_same_v<T, std::monostate>)
                    {
                        spdlog::debug("Adding component {} to entity {} of type {}",
                                      typeid(comp).name(), entity, entityType);
                        gameState->addComponent(entity, comp);
                    }
                },
                variantComponent);
        }
    }
    gameState->getComponent<CompEntityInfo>(entity).entityId = entity;

    if (gameState->hasComponent<CompUnit>(entity))
        gameState->getComponent<CompUnit>(entity).commandQueue.push(
            ObjectPool<CmdIdle>::acquire(entity));

    return entity;
}

void EntityDefinitionLoader::createOrUpdateComponent(py::object module,
                                                     uint32_t entityType,
                                                     pybind11::handle entityDefinition)
{
    py::dict fields = entityDefinition.attr("__dict__");

    addComponentIfNotNull(entityType, createAnimation(module, entityDefinition));
    addComponentIfNotNull(entityType, createBuilder(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompResourceGatherer(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompUnit(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompTransform(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompResource(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompBuilding(module, entityDefinition));
    addComponentIfNotNull(entityType, CompEntityInfo(entityType));
}

void EntityDefinitionLoader::addComponentsForUnit(uint32_t entityType)
{
    m_componentsByEntityType[entityType].push_back(CompRendering());
    m_componentsByEntityType[entityType].push_back(CompAction(UnitAction::IDLE));
    m_componentsByEntityType[entityType].push_back(CompDirty());
    m_componentsByEntityType[entityType].push_back(CompPlayer());
    // TODO: Following is not correct, need to populate
    m_componentsByEntityType[entityType].push_back(CompSelectible());

    CompGraphics graphics;
    graphics.entityType = entityType;
    graphics.layer = GraphicLayer::ENTITIES;
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::GREEN,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::RED,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::BLUE,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::YELLOW,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, ion::Color::BLACK,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});
    m_componentsByEntityType[entityType].push_back(graphics);
}

void EntityDefinitionLoader::addComponentIfNotNull(uint32_t entityType, const ComponentType& comp)
{
    if (std::holds_alternative<std::monostate>(comp) == false)
        m_componentsByEntityType[entityType].push_back(comp);
}

Ref<DRSFile> loadDRSFile2(const string& drsFilename)
{
    auto drs = make_shared<DRSFile>();
    if (!drs->load(drsFilename))
    {
        throw runtime_error("Failed to load DRS file: " + drsFilename);
    }
    return drs;
}

void EntityDefinitionLoader::updateDRSData(uint32_t entityType, pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "animations"))
    {
        for (auto py_anim : entityDefinition.attr("animations"))
        {
            auto drsFileName = EntityDefinitionLoader::readValue<std::string>(py_anim, "drs_file");
            auto slpId = EntityDefinitionLoader::readValue<int>(py_anim, "slp_id");

            Ref<DRSFile> drsFile;
            auto it = m_drsFilesByName.find(drsFileName);
            if (it != m_drsFilesByName.end())
                drsFile = it->second;
            else
                drsFile = loadDRSFile2("assets/" + drsFileName);

            auto action = getAction(py_anim.attr("name").cast<std::string>());

            GraphicsID id;
            id.entityType = entityType;
            id.action = action;

            m_DRSDataByGraphicsIdHash[id.hash()] = EntityDRSData{drsFile, slpId};
        }
    }

    if (py::hasattr(entityDefinition, "graphics"))
    {
        py::dict graphicsDict = entityDefinition.attr("graphics");

        for (auto [theme, graphicsEntry] : graphicsDict)
        {
            auto drsFileName = EntityDefinitionLoader::readValue<std::string>(graphicsEntry, "drs_file");
            auto slpId = EntityDefinitionLoader::readValue<int>(graphicsEntry, "slp_id");

            Ref<DRSFile> drsFile;
            auto it = m_drsFilesByName.find(drsFileName);
            if (it != m_drsFilesByName.end())
                drsFile = it->second;
            else
                drsFile = loadDRSFile2("assets/" + drsFileName);

            GraphicsID id;
            id.entityType = entityType;
            m_DRSDataByGraphicsIdHash[id.hash()] = EntityDRSData{drsFile, slpId};
        }
    }
}
