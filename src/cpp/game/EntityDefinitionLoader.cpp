#include "EntityDefinitionLoader.h"

#include "GameTypes.h"
#include "commands/CmdIdle.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

#include <pybind11/stl.h>
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
        {"construction_site", EntityTypes::ET_CONSTRUCTION_SITE},
    };
    return entityTypes.at(entityName);
}

uint32_t getEntitySubType(uint32_t entityType, const std::string& name)
{
    if (entityType == EntityTypes::ET_CONSTRUCTION_SITE)
    {
        if (name == "small")
            return EntitySubTypes::EST_SMALL_SIZE;
        else if (name == "medium")
            return EntitySubTypes::EST_MEDIUM_SIZE;
    }
    else if (entityType == EntityTypes::ET_TREE)
    {
        if (name == "stump")
            return EntitySubTypes::EST_CHOPPED_TREE;
        if (name == "shadow")
            return EntitySubTypes::EST_TREE_SHADOW;
    }
    return EntitySubTypes::EST_DEFAULT;
}

UIElements getUIElement(const std::string& name)
{
    if (name == "resource_panel")
        return UIElements::UI_ELEMENT_RESOURCE_PANEL;
    else if (name == "info_panel")
        return UIElements::UI_ELEMENT_INFO_PANEL;
    return UIElements::UI_ELEMENT_UNKNOWN;
}

ResourceType getResourceType(const std::string& name)
{
    static unordered_map<string, ResourceType> resTypes = {
        {"wood", ResourceType::WOOD},
        {"gold", ResourceType::GOLD},
        {"stone", ResourceType::STONE},
        {"food", ResourceType::FOOD},
    };
    return resTypes.at(name);
}

Size getBuildingSize(const std::string& name)
{
    if (name == "small")
        return Size(1, 1);
    else if (name == "medium")
        return Size(2, 2);
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

        PropertyInitializer::set(comp.animations[action], animation);
    }
    return ComponentType(comp);
}

ComponentType EntityDefinitionLoader::createBuilder(py::object module,
                                                    pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "build_speed") == false)
        return std::monostate{};

    auto buildSpeed = entityDefinition.attr("build_speed").cast<int>();
    CompBuilder builder;
    PropertyInitializer::set<uint32_t>(builder.buildSpeed, buildSpeed);
    return ComponentType(builder);
}

ComponentType EntityDefinitionLoader::createCompResourceGatherer(py::object module,
                                                                 pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "gather_speed") == false)
        return std::monostate{};

    CompResourceGatherer comp;
    PropertyInitializer::set<uint32_t>(comp.gatherSpeed,
                                       entityDefinition.attr("gather_speed").cast<int>());
    PropertyInitializer::set<uint32_t>(comp.capacity,
                                       entityDefinition.attr("resource_capacity").cast<int>());
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
            PropertyInitializer::set<uint32_t>(comp.lineOfSight,
                                               entityDefinition.attr("line_of_sight").cast<int>());
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
    PropertyInitializer::set<uint32_t>(comp.speed,
                                       entityDefinition.attr("moving_speed").cast<int>());
    PropertyInitializer::set(comp.hasRotation, true);
    return ComponentType(comp);
}

ComponentType EntityDefinitionLoader::createCompResource(py::object module,
                                                         pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "resource_amount") == false)
        return std::monostate{};

    auto name = EntityDefinitionLoader::readValue<string>(entityDefinition, "name");
    auto amount = EntityDefinitionLoader::readValue<int>(entityDefinition, "resource_amount");
    CompResource comp;
    PropertyInitializer::set<Resource>(comp.original, Resource(getResourceType(name), amount));
    comp.remainingAmount = comp.original.value().amount;

    return ComponentType(comp);
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

            auto lineOfSight = readValue<int>(entityDefinition, "line_of_sight");
            auto sizeStr = readValue<std::string>(entityDefinition, "size");
            auto size = getBuildingSize(sizeStr);

            PropertyInitializer::set<uint32_t>(comp.lineOfSight, lineOfSight);
            PropertyInitializer::set<Size>(comp.size, size);

            if (py::hasattr(module, "ResourceDropOff"))
            {
                py::object dropOffClass = module.attr("ResourceDropOff");
                if (py::isinstance(entityDefinition, dropOffClass))
                {
                    auto acceptedResources =
                        readValue<std::list<std::string>>(entityDefinition, "accepted_resources");

                    uint8_t acceptedResourceFlag = 0;
                    for (auto& resStr : acceptedResources)
                    {
                        auto resType = getResourceType(resStr);
                        acceptedResourceFlag |= resType;
                    }
                    PropertyInitializer::set(comp.dropOffForResourceType, acceptedResourceFlag);
                }
            }
            return ComponentType(comp);
        }
    }
    return std::monostate{};
}

EntityDefinitionLoader::EntityDefinitionLoader(/* args */)
{
    m_drsLoadFunc = [](const std::string& drsFilename) -> Ref<DRSFile>
    {
        auto drs = std::make_shared<DRSFile>();
        if (!drs->load(drsFilename))
        {
            throw std::runtime_error("Failed to load DRS file: " + drsFilename);
        }
        return drs;
    };
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
    loadConstructionSites(module);
    loadBuildings(module);
    loadTileSets(module);
    loadUIElements(module);
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

            py::object treeClass = module.attr("Tree");
            if (py::isinstance(entry, treeClass))
            {
                if (py::hasattr(entry, "stump"))
                {
                    auto stumpSubType = getEntitySubType(entityType, "stump");
                    py::object stump = entry.attr("stump");
                    updateDRSData(entityType, stumpSubType, 0, stump);
                }

                if (py::hasattr(entry, "shadow"))
                {
                    auto shadowSubType = getEntitySubType(entityType, "shadow");
                    py::object stump = entry.attr("shadow");
                    updateDRSData(entityType, shadowSubType, 0, stump);
                }
            }
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
            std::string size = entry.attr("size").cast<std::string>();

            auto entityType = getEntityType(name);
            addComponentsForBuilding(entityType);
            createOrUpdateComponent(module, entityType, entry);
            updateDRSData(entityType, entry);
            attachedConstructionSites(entityType, size);
        }
    }
}

void EntityDefinitionLoader::loadConstructionSites(pybind11::object module)
{
    if (py::hasattr(module, "all_construction_sites"))
    {
        py::list entries = module.attr("all_construction_sites");

        for (auto entry : entries)
        {
            std::string name = entry.attr("name").cast<std::string>();
            ConstructionSiteData data;
            auto sizeStr = readValue<std::string>(entry, "size");
            data.size = getBuildingSize(sizeStr);
            data.progressToFrames = entry.attr("progress_frame_map").cast<std::map<int, int>>();
            m_constructionSitesBySize[sizeStr] = data;

            auto entityType = getEntityType(name);
            auto entitySubType = getEntitySubType(entityType, sizeStr);
            updateDRSData(entityType, entitySubType, 0, entry);
        }
    }
}

void EntityDefinitionLoader::loadTileSets(pybind11::object module)
{
    if (py::hasattr(module, "all_tilesets"))
    {
        py::list entries = module.attr("all_tilesets");

        for (auto entry : entries)
        {
            updateDRSData(EntityTypes::ET_TILE, entry);
        }
    }
}

void EntityDefinitionLoader::loadUIElements(pybind11::object module)
{
    if (py::hasattr(module, "all_ui_elements"))
    {
        py::list entries = module.attr("all_ui_elements");

        for (auto entry : entries)
        {
            auto name = readValue<std::string>(entry, "name");

            auto element = getUIElement(name);
            updateDRSData(EntityTypes::ET_UI_ELEMENT, EntitySubTypes::UI_WINDOW, element, entry);
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
    {
        auto idleCmd = ObjectPool<CmdIdle>::acquire(entity);
        idleCmd->setPriority(Command::DEFAULT_PRIORITY);
        gameState->getComponent<CompUnit>(entity).commandQueue.push(idleCmd);
    }
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

void EntityDefinitionLoader::addComponentsForBuilding(uint32_t entityType)
{
    m_componentsByEntityType[entityType].push_back(CompRendering());
    m_componentsByEntityType[entityType].push_back(CompDirty());
    m_componentsByEntityType[entityType].push_back(CompTransform());
    m_componentsByEntityType[entityType].push_back(CompPlayer());

    CompGraphics graphics;
    graphics.entityType = entityType;
    graphics.layer = GraphicLayer::ENTITIES;
    m_componentsByEntityType[entityType].push_back(graphics);
}

void EntityDefinitionLoader::addComponentIfNotNull(uint32_t entityType, const ComponentType& comp)
{
    if (std::holds_alternative<std::monostate>(comp) == false)
        m_componentsByEntityType[entityType].push_back(comp);
}

void EntityDefinitionLoader::updateDRSData(uint32_t entityType, pybind11::handle entityDefinition)
{
    updateDRSData(entityType, EntitySubTypes::EST_DEFAULT, 0, entityDefinition);
}

void EntityDefinitionLoader::updateDRSData(uint32_t entityType,
                                           uint32_t entitySubType,
                                           uint32_t action,
                                           pybind11::handle entityDefinition)
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
                drsFile = m_drsLoadFunc("assets/" + drsFileName);

            auto action = getAction(py_anim.attr("name").cast<std::string>());

            GraphicsID id;
            id.entityType = entityType;
            id.entitySubType = entitySubType;
            id.action = action;

            m_DRSDataByGraphicsIdHash[id.hash()] = EntityDRSData{drsFile, slpId};
        }
    }

    if (py::hasattr(entityDefinition, "graphics"))
    {
        py::dict graphicsDict = entityDefinition.attr("graphics");

        for (auto [theme, graphicsEntry] : graphicsDict)
        {
            auto drsFileName =
                EntityDefinitionLoader::readValue<std::string>(graphicsEntry, "drs_file");
            auto slpId = EntityDefinitionLoader::readValue<int>(graphicsEntry, "slp_id");

            Ref<DRSFile> drsFile;
            auto it = m_drsFilesByName.find(drsFileName);
            if (it != m_drsFilesByName.end())
                drsFile = it->second;
            else
                drsFile = m_drsLoadFunc("assets/" + drsFileName);

            Rect<int> clipRect;
            if (py::hasattr(graphicsEntry, "clip_rect"))
            {
                py::object rect = graphicsEntry.attr("clip_rect");
                clipRect.x = readValue<int>(rect, "x");
                clipRect.y = readValue<int>(rect, "y");
                clipRect.w = readValue<int>(rect, "w");
                clipRect.h = readValue<int>(rect, "h");
            }

            GraphicsID id;
            id.entityType = entityType;
            id.entitySubType = entitySubType;
            id.action = action;
            m_DRSDataByGraphicsIdHash[id.hash()] = EntityDRSData{drsFile, slpId, clipRect};
        }
    }
}

EntityDefinitionLoader::ConstructionSiteData EntityDefinitionLoader::getSite(
    const std::string& sizeStr)
{
    return m_constructionSitesBySize.at(sizeStr);
}

void EntityDefinitionLoader::setSite(const std::string& sizeStr,
                                     const std::map<int, int>& progressToFrames)
{
    m_constructionSitesBySize[sizeStr] = ConstructionSiteData{.progressToFrames = progressToFrames};
}

void EntityDefinitionLoader::attachedConstructionSites(uint32_t entityType,
                                                       const std::string& sizeStr)
{
    // Approach: Find and attach construction site variants to the original
    // building component for the entity type.
    auto& components = m_componentsByEntityType.at(entityType);
    auto siteData = m_constructionSitesBySize.at(sizeStr);

    for (auto& comp : components)
    {
        if (std::holds_alternative<CompBuilding>(comp))
        {
            auto& buildingComp = std::get<CompBuilding>(comp);
            buildingComp.constructionSiteEntitySubType =
                getEntitySubType(EntityTypes::ET_CONSTRUCTION_SITE, sizeStr);
            buildingComp.visualVariationByProgress = siteData.progressToFrames;
            // Fallback to the main variation if the building progress is 100. This is used
            // to switch to the main building texture instead of the construction site texture
            // using entitySubType.
            // So when entitySubType is resetted to default to display the main building, we
            // need to default the variation also.
            buildingComp.visualVariationByProgress[100] = 0;
            break;
        }
    }

    // Approach: Find the construction site DRS data and attach it to building entity
    // as entity sub types.
    GraphicsID siteId;
    siteId.entityType = EntityTypes::ET_CONSTRUCTION_SITE;
    siteId.entitySubType = getEntitySubType(EntityTypes::ET_CONSTRUCTION_SITE, sizeStr);
    auto siteDRSData = m_DRSDataByGraphicsIdHash.at(siteId.hash());

    GraphicsID buildingAttachedSiteId;
    buildingAttachedSiteId.entityType = entityType;
    buildingAttachedSiteId.entitySubType = siteId.entitySubType;
    m_DRSDataByGraphicsIdHash[buildingAttachedSiteId.hash()] = siteDRSData;
    ;
}

void EntityDefinitionLoader::setDRSData(int64_t id, const EntityDRSData& data)
{
    m_DRSDataByGraphicsIdHash[id] = data;
}

void EntityDefinitionLoader::setDRSLoaderFunc(std::function<Ref<DRSFile>(const std::string&)> func)
{
    m_drsLoadFunc = func;
}
