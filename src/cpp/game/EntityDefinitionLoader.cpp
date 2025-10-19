#include "EntityDefinitionLoader.h"

#include "EntityTypeRegistry.h"
#include "GameTypes.h"
#include "commands/CmdIdle.h"
#include "components/CompGarrison.h"
#include "components/CompHousing.h"
#include "components/CompUnitFactory.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

#include <pybind11/stl.h>
#include <unordered_map>

using namespace core;
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

uint32_t getEntitySubType(uint32_t entityType, const std::string& name)
{
    if (entityType == EntityTypes::ET_CONSTRUCTION_SITE)
    {
        if (name == "small")
            return EntitySubTypes::EST_SMALL_SIZE;
        else if (name == "medium")
            return EntitySubTypes::EST_MEDIUM_SIZE;
        else if (name == "large")
            return EntitySubTypes::EST_LARGE_SIZE;
        else if (name == "huge")
            return EntitySubTypes::EST_HUGE_SIZE;
        else
            spdlog::warn("Unknown subtype {} for entity type {}", name, entityType);
    }
    else if (entityType == EntityTypes::ET_TREE)
    {
        if (name == "stump")
            return EntitySubTypes::EST_CHOPPED_TREE;
        else if (name == "shadow")
            return EntitySubTypes::EST_TREE_SHADOW;
        else
            spdlog::warn("Unknown subtype {} for entity type {}", name, entityType);
    }
    else if (entityType == EntityTypes::ET_UI_ELEMENT)
    {
        if (name == "resource_panel")
            return EntitySubTypes::EST_UI_RESOURCE_PANEL;
        else if (name == "control_panel")
            return EntitySubTypes::EST_UI_CONTROL_PANEL;
        else if (name == "progress_bar")
            return EntitySubTypes::UI_PROGRESS_BAR;
        else if (name == "cursor")
            return EntitySubTypes::UI_CURSOR;
    }
    return EntitySubTypes::EST_DEFAULT;
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
    else if (name == "large")
        return Size(3, 3);
    else if (name == "huge")
        return Size(4, 4);
    else
    {
        debug_assert(false, "unknown building size");
        return Size(0, 0);
    }
}

Rect<int> getBoundingBox(shared_ptr<DRSFile> drs, uint32_t slpId)
{
    auto frameInfos = drs->getSLPFile(slpId).getFrameInfos();
    auto frame = frameInfos[0];
    Rect<int> box(frame.hotspot_x, frame.hotspot_y, frame.width, frame.height);
    return box;
}

void validatePython(py::module_ sysModule)
{
    auto paths = sysModule.attr("path");
    std::vector<std::string> pathVec;
    spdlog::info("Python sys.path:");

    for (auto item : paths)
    {
        spdlog::info("  {}", item.cast<std::string>());
    }

    // Try importing pydantic
    py::module_ pydantic = py::module_::import("pydantic");
    spdlog::info("Pydantic is found");

    // Simple pydantic BaseModel test
    py::exec(R"(
from pydantic import BaseModel, ValidationError

class MyModel(BaseModel):
    x: int

# valid instance
m = MyModel(x=10)
# invalid instance to see validation
try:
    m2 = MyModel(x="not an int")
except ValidationError as e:
    print("Validated pybind setup")
)");
}

EntityDefinitionLoader::EntityDefinitionLoader()
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

    m_boundingBoxReadFunc = getBoundingBox;
}

EntityDefinitionLoader::~EntityDefinitionLoader()
{
}

void EntityDefinitionLoader::load()
{
    py::scoped_interpreter guard{};

    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("insert")(0, "./assets/scripts");

    validatePython(sys);

    py::object module = py::module_::import("importer");
    validateEntities(module);
    loadEntityTypes(module);
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
        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

        for (auto py_unit : all_units)
        {
            std::string name = py_unit.attr("name").cast<std::string>();

            if (typeRegistry->isValid(name))
            {
                auto entityType = typeRegistry->getEntityType(name);
                typeRegistry->registerUnitType(entityType);

                addComponentsForUnit(entityType);
                addCommonComponents(module, entityType, py_unit);
                loadDRSForAnimations(entityType, EntitySubTypes::EST_DEFAULT, py_unit);

                GraphicsID id;
                id.entityType = entityType;
                auto drsData = m_DRSDataByGraphicsIdHash.at(id);

                auto& sc = getComponent<CompSelectible>(entityType, EntitySubTypes::EST_DEFAULT);
                for (int i = static_cast<int>(Direction::NORTH);
                     i <= static_cast<int>(Direction::NORTHWEST); ++i)
                {
                    sc.boundingBoxes[i] = drsData.boundingRect;
                }
            }
            else
            {
                spdlog::error("Invalid entity {}, skipping loading", name);
            }
        }
    }
}

void EntityDefinitionLoader::loadNaturalResources(py::object module)
{
    if (py::hasattr(module, "all_natural_resources"))
    {
        py::list entries = module.attr("all_natural_resources");
        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

        for (auto entry : entries)
        {
            std::string name = entry.attr("name").cast<std::string>();

            if (typeRegistry->isValid(name))
            {
                auto entityType = typeRegistry->getEntityType(name);

                addComponentsForNaturalResource(entityType);
                addCommonComponents(module, entityType, entry);
                loadDRSForStillImage(module, entityType, EntitySubTypes::EST_DEFAULT, entry);

                GraphicsID id;
                id.entityType = entityType;
                auto drsData = m_DRSDataByGraphicsIdHash.at(id);

                auto& sc = getComponent<CompSelectible>(entityType, EntitySubTypes::EST_DEFAULT);
                sc.boundingBoxes[static_cast<int>(Direction::NONE)] = drsData.boundingRect;

                py::object treeClass = module.attr("Tree");
                if (py::isinstance(entry, treeClass))
                {
                    if (py::hasattr(entry, "stump"))
                    {
                        auto stumpSubType = getEntitySubType(entityType, "stump");
                        py::object stump = entry.attr("stump");
                        loadDRSForStillImage(module, entityType, stumpSubType, stump);
                    }

                    if (py::hasattr(entry, "shadow"))
                    {
                        auto shadowSubType = getEntitySubType(entityType, "shadow");
                        py::object stump = entry.attr("shadow");
                        loadDRSForStillImage(module, entityType, shadowSubType, stump);

                        CompGraphics graphics;
                        graphics.layer = GraphicLayer::ON_GROUND;

                        CompEntityInfo info(entityType);
                        info.entitySubType = shadowSubType;

                        auto entityTypeAndSubType =
                            entityType + m_entitySubTypeMapKeyOffset * shadowSubType;

                        addComponentIfNotNull(entityTypeAndSubType, graphics);
                        addComponentIfNotNull(entityTypeAndSubType, info);
                        addComponentIfNotNull(entityTypeAndSubType, CompRendering());
                        addComponentIfNotNull(entityTypeAndSubType, CompTransform());
                        addComponentIfNotNull(entityTypeAndSubType, CompDirty());
                    }
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
        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

        for (auto entry : entries)
        {
            std::string name = entry.attr("name").cast<std::string>();
            std::string size = entry.attr("size").cast<std::string>();

            if (typeRegistry->isValid(name))
            {
                auto entityType = typeRegistry->getEntityType(name);

                addComponentsForBuilding(entityType);
                addCommonComponents(module, entityType, entry);
                loadDRSForStillImage(module, entityType, EntitySubTypes::EST_DEFAULT, entry);
                attachedConstructionSites(entityType, size);

                GraphicsID id;
                id.entityType = entityType;
                auto drsData = m_DRSDataByGraphicsIdHash.at(id);

                auto& sc = getComponent<CompSelectible>(entityType, EntitySubTypes::EST_DEFAULT);
                sc.boundingBoxes[static_cast<int>(Direction::NONE)] = drsData.boundingRect;
            }
            else
            {
                spdlog::error("Invalid entity {}, skipping loading", name);
            }
        }
    }
}

void EntityDefinitionLoader::loadConstructionSites(pybind11::object module)
{
    if (py::hasattr(module, "all_construction_sites"))
    {
        py::list entries = module.attr("all_construction_sites");
        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

        for (auto entry : entries)
        {
            std::string name = entry.attr("name").cast<std::string>();
            ConstructionSiteData data;
            auto sizeStr = readValue<std::string>(entry, "size");
            data.size = getBuildingSize(sizeStr);
            data.progressToFrames = entry.attr("progress_frame_map").cast<std::map<int, int>>();
            m_constructionSitesBySize[sizeStr] = data;

            auto entityType = typeRegistry->getEntityType(name);

            auto entitySubType = getEntitySubType(entityType, sizeStr);
            loadDRSForStillImage(module, entityType, entitySubType, entry);
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
            addComponentsForTileset(EntityTypes::ET_TILE);
            loadDRSForStillImage(module, EntityTypes::ET_TILE, EntitySubTypes::EST_DEFAULT, entry);
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

            auto subType = getEntitySubType(EntityTypes::ET_UI_ELEMENT, name);
            loadDRSForStillImage(module, EntityTypes::ET_UI_ELEMENT, subType, entry);
        }
    }
}

EntityDefinitionLoader::EntityDRSData EntityDefinitionLoader::getDRSData(const GraphicsID& id)
{
    auto it = m_DRSDataByGraphicsIdHash.find(id);
    if (it != m_DRSDataByGraphicsIdHash.end())
        return it->second;
    else
    {
        for (auto& e : m_DRSDataByGraphicsIdHash)
        {
            spdlog::debug("getDRSData: map {}:{}", e.first.toString(), e.second.parts[0].slpId);
        }
        spdlog::warn("Could not find DRS data for id {}", id.toString());
    }
    return {};
}

uint32_t EntityDefinitionLoader::createEntity(uint32_t entityType, uint32_t entitySubType)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto entity = stateMan->createEntity();

    auto mapKey = entityType + entitySubType * m_entitySubTypeMapKeyOffset;

    auto it = m_componentsByEntityType.find(mapKey);
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
                        spam("Adding component {} to entity {} of type {} subtype {}",
                             typeid(comp).name(), entity, entityType, entitySubType);
                        stateMan->addComponent(entity, comp);
                    }
                },
                variantComponent);
        }
    }
    auto& info = stateMan->getComponent<CompEntityInfo>(entity);
    PropertyInitializer::set(info.entityId, entity);

    // Adding default idle command
    if (stateMan->hasComponent<CompUnit>(entity))
    {
        auto idleCmd = ObjectPool<CmdIdle>::acquire(entity);
        idleCmd->setPriority(Command::DEFAULT_PRIORITY);
        stateMan->getComponent<CompUnit>(entity).commandQueue.push(idleCmd);
    }

    // Late binding of entity names to types
    if (stateMan->hasComponent<CompUnitFactory>(entity))
    {
        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

        std::vector<uint32_t> possibleUnitTypes;
        auto& factory = stateMan->getComponent<CompUnitFactory>(entity);
        for (auto& possibleUnit : factory.producibleUnitNames.value())
        {
            if (typeRegistry->isValid(possibleUnit))
            {
                possibleUnitTypes.push_back(typeRegistry->getEntityType(possibleUnit));
            }
        }

        std::unordered_map<char, uint32_t> entityTypesByShortcut;
        for (auto [key, name] : factory.producibleUnitNamesByShortcuts.value())
        {
            if (typeRegistry->isValid(name))
            {
                entityTypesByShortcut[key] = typeRegistry->getEntityType(name);
            }
        }
        PropertyInitializer::set(factory.producibleUnitTypes, possibleUnitTypes);
        PropertyInitializer::set(factory.producibleUnitShortcuts, entityTypesByShortcut);
    }

    // Late binding of entity names to types
    if (auto builder = stateMan->tryGetComponent<CompBuilder>(entity))
    {
        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

        std::unordered_map<char, uint32_t> entityTypesByShortcut;
        for (auto& [key, name] : builder->buildableNameByShortcut.value())
        {
            if (typeRegistry->isValid(name))
            {
                entityTypesByShortcut[key] = typeRegistry->getEntityType(name);
            }
        }
        PropertyInitializer::set(builder->buildableTypesByShortcut, entityTypesByShortcut);
    }
    return entity;
}

void EntityDefinitionLoader::addCommonComponents(py::object module,
                                                 uint32_t entityType,
                                                 pybind11::handle entityDefinition)
{
    py::dict fields = entityDefinition.attr("__dict__");

    addComponentIfNotNull(entityType, createAnimation(module, entityDefinition));
    addComponentIfNotNull(entityType, createBuilder(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompResourceGatherer(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompUnit(entityType, module, entityDefinition));
    addComponentIfNotNull(entityType, createCompTransform(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompResource(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompBuilding(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompUnitFactory(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompHousing(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompGarrison(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompSelectible(entityType, module, entityDefinition));
    addComponentIfNotNull(entityType, CompEntityInfo(entityType));
}

void EntityDefinitionLoader::addComponentsForUnit(uint32_t entityType)
{
    CompGraphics graphics;
    graphics.layer = GraphicLayer::ENTITIES;
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, core::Color::GREEN,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, core::Color::RED,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, core::Color::BLUE,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, core::Color::YELLOW,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});
    graphics.debugOverlays.push_back({DebugOverlay::Type::ARROW, core::Color::BLACK,
                                      DebugOverlay::FixedPosition::BOTTOM_CENTER,
                                      DebugOverlay::FixedPosition::CENTER});

    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompAction(UnitAction::IDLE));
    addComponentIfNotNull(entityType, CompDirty());
    addComponentIfNotNull(entityType, CompPlayer());
    addComponentIfNotNull(entityType, graphics);
}

void EntityDefinitionLoader::addComponentsForBuilding(uint32_t entityType)
{
    CompGraphics graphics;
    graphics.layer = GraphicLayer::ENTITIES;

    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompDirty());
    addComponentIfNotNull(entityType, CompPlayer());
    addComponentIfNotNull(entityType, CompTransform());
    addComponentIfNotNull(entityType, graphics);
}

void EntityDefinitionLoader::addComponentsForNaturalResource(uint32_t entityType)
{
    CompGraphics graphics;
    graphics.layer = GraphicLayer::ENTITIES;

    addComponentIfNotNull(entityType, graphics);
    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompTransform());
    addComponentIfNotNull(entityType, CompDirty());
}

void EntityDefinitionLoader::addComponentsForTileset(uint32_t entityType)
{
    CompGraphics graphics;
    graphics.layer = GraphicLayer::GROUND;
    DebugOverlay overlay{DebugOverlay::Type::RHOMBUS, core::Color::GREY,
                         DebugOverlay::FixedPosition::BOTTOM_CENTER};
    overlay.customPos1 = DebugOverlay::FixedPosition::CENTER_LEFT;
    overlay.customPos2 = DebugOverlay::FixedPosition::CENTER_RIGHT;
    graphics.debugOverlays.push_back(overlay);

    addComponentIfNotNull(entityType, graphics);
    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompTransform());
    addComponentIfNotNull(entityType, CompEntityInfo(entityType));
    addComponentIfNotNull(entityType, CompDirty());
}

void EntityDefinitionLoader::addComponentIfNotNull(uint32_t entityType, const ComponentType& comp)
{
    if (std::holds_alternative<std::monostate>(comp) == false)
        m_componentsByEntityType[entityType].push_back(comp);
}

void EntityDefinitionLoader::loadDRSForAnimations(uint32_t entityType,
                                                  uint32_t entitySubType,
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

            // TODO: Bounding box might have to change depending on the direction
            auto boundingBox = m_boundingBoxReadFunc(drsFile, slpId);

            EntityDRSData data;
            data.parts.push_back(EntityDRSData::Part(drsFile, slpId, Vec2::null));
            data.boundingRect = boundingBox;

            m_DRSDataByGraphicsIdHash[id] = data;
        }
    }
}

void EntityDefinitionLoader::loadDRSForStillImage(pybind11::object module,
                                                  uint32_t entityType,
                                                  uint32_t entitySubType,
                                                  pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "graphics"))
    {
        py::dict graphicsDict = entityDefinition.attr("graphics");
        py::object compositeGraphicClass = module.attr("CompositeGraphic");

        for (auto [theme, graphicsEntry] : graphicsDict)
        {
            auto processGraphic = [&](py::handle graphicObj, const Vec2& anchor)
            {
                // TODO: handle theme
                auto drsFileName =
                    EntityDefinitionLoader::readValue<std::string>(graphicObj, "drs_file");
                auto slpId = EntityDefinitionLoader::readValue<int>(graphicObj, "slp_id");

                Vec2 anchorOverride = Vec2::null;
                if (py::hasattr(graphicObj, "anchor"))
                {
                    py::object anchor = graphicObj.attr("anchor");
                    anchorOverride.x = readValue<int>(anchor, "x");
                    anchorOverride.y = readValue<int>(anchor, "y");
                }

                Ref<DRSFile> drsFile;
                auto it = m_drsFilesByName.find(drsFileName);
                if (it != m_drsFilesByName.end())
                    drsFile = it->second;
                else
                    drsFile = m_drsLoadFunc("assets/" + drsFileName);

                Rect<int> clipRect;
                if (py::hasattr(graphicObj, "clip_rect"))
                {
                    py::object rect = graphicObj.attr("clip_rect");
                    clipRect.x = readValue<int>(rect, "x");
                    clipRect.y = readValue<int>(rect, "y");
                    clipRect.w = readValue<int>(rect, "w");
                    clipRect.h = readValue<int>(rect, "h");
                }

                GraphicsID id;
                id.entityType = entityType;
                id.entitySubType = entitySubType;

                auto boundingBox = m_boundingBoxReadFunc(drsFile, slpId);

                if (m_DRSDataByGraphicsIdHash.contains(id))
                {
                    m_DRSDataByGraphicsIdHash[id].parts.push_back(
                        EntityDRSData::Part(drsFile, slpId, anchorOverride));
                }
                else
                {
                    EntityDRSData data;
                    data.parts.push_back(EntityDRSData::Part(drsFile, slpId, anchorOverride));
                    data.boundingRect = boundingBox;
                    data.clipRect = clipRect;
                    data.anchor = anchor;

                    m_DRSDataByGraphicsIdHash[id] = data;
                }

                spdlog::debug("Loading DRS data for id {}, slp id {}", id.toString(), slpId);
            };

            if (py::isinstance(graphicsEntry, compositeGraphicClass))
            {
                py::list parts = graphicsEntry.attr("parts");
                py::object anchor = graphicsEntry.attr("anchor");
                int anchorX = readValue<int>(anchor, "x");
                int anchorY = readValue<int>(anchor, "y");

                for (auto part : parts)
                    processGraphic(part, Vec2(anchorX, anchorY));
            }
            else
            {
                for (auto part : graphicsEntry)
                    processGraphic(part, Vec2::null);
            }
        }
    }
}

void EntityDefinitionLoader::loadDRSForIcon(uint32_t entityType,
                                            uint32_t entitySubType,
                                            pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "drs_file"))
    {
        auto drsFileName =
            EntityDefinitionLoader::readValue<std::string>(entityDefinition, "drs_file");
        auto slpId = EntityDefinitionLoader::readValue<int>(entityDefinition, "slp_id");

        Ref<DRSFile> drsFile;
        auto it = m_drsFilesByName.find(drsFileName);
        if (it != m_drsFilesByName.end())
            drsFile = it->second;
        else
            drsFile = m_drsLoadFunc("assets/" + drsFileName);

        Rect<int> clipRect;
        if (py::hasattr(entityDefinition, "clip_rect"))
        {
            py::object rect = entityDefinition.attr("clip_rect");
            clipRect.x = readValue<int>(rect, "x");
            clipRect.y = readValue<int>(rect, "y");
            clipRect.w = readValue<int>(rect, "w");
            clipRect.h = readValue<int>(rect, "h");
        }

        GraphicsID id;
        id.entityType = entityType;
        id.entitySubType = entitySubType;

        EntityDRSData data;
        data.parts.push_back(EntityDRSData::Part(drsFile, slpId, Vec2::null));
        data.clipRect = clipRect;

        m_DRSDataByGraphicsIdHash[id] = data;
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
    auto siteDRSData = m_DRSDataByGraphicsIdHash.at(siteId);

    GraphicsID buildingAttachedSiteId;
    buildingAttachedSiteId.entityType = entityType;
    buildingAttachedSiteId.entitySubType = siteId.entitySubType;
    m_DRSDataByGraphicsIdHash[buildingAttachedSiteId] = siteDRSData;
    ;
}

void EntityDefinitionLoader::setDRSData(const GraphicsID& id, const EntityDRSData& data)
{
    m_DRSDataByGraphicsIdHash[id] = data;
}

void EntityDefinitionLoader::setDRSLoaderFunc(std::function<Ref<DRSFile>(const std::string&)> func)
{
    m_drsLoadFunc = func;
}

GraphicsID EntityDefinitionLoader::readIconDef(uint32_t entitySubType,
                                               pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "icon"))
    {
        py::object iconDef = entityDefinition.attr("icon");
        auto index = EntityDefinitionLoader::readValue<int>(iconDef, "index");

        GraphicsID icon;
        icon.entityType = EntityTypes::ET_UI_ELEMENT;
        icon.entitySubType = entitySubType;
        icon.variation = index;

        loadDRSForIcon(EntityTypes::ET_UI_ELEMENT, entitySubType, iconDef);
        return icon;
    }
    return {};
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

    py::object buildables = entityDefinition.attr("buildables");
    std::unordered_map<char, std::string> entityNameByShortcut;

    for (auto building : buildables)
    {
        auto name = readValue<std::string>(building, "name");
        auto shortcut = readValue<std::string>(building, "shortcut");
        char key = 0;
        if (shortcut.empty() == false)
        {
            key = shortcut.c_str()[0];
        }
        entityNameByShortcut[key] = name;
    }

    auto buildSpeed = entityDefinition.attr("build_speed").cast<int>();
    CompBuilder builder;
    PropertyInitializer::set<uint32_t>(builder.buildSpeed, buildSpeed);
    PropertyInitializer::set<std::unordered_map<char, std::string>>(builder.buildableNameByShortcut,
                                                                    entityNameByShortcut);
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

ComponentType EntityDefinitionLoader::createCompUnit(uint32_t entityType,
                                                     py::object module,
                                                     pybind11::handle entityDefinition)
{
    if (py::hasattr(module, "Unit"))
    {
        py::object unitClass = module.attr("Unit");
        if (py::isinstance(entityDefinition, unitClass))
        {
            uint32_t unitTypeInt = entityDefinition.attr("unit_type").cast<int>();
            auto unitType = getUnitType(unitTypeInt);

            CompUnit comp;
            PropertyInitializer::set<uint32_t>(comp.lineOfSight,
                                               entityDefinition.attr("line_of_sight").cast<int>());
            PropertyInitializer::set<uint32_t>(comp.housingNeed,
                                               entityDefinition.attr("housing_need").cast<int>());
            PropertyInitializer::set<UnitType>(comp.type, unitType);

            auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
            typeRegistry->registerUnitTypeHousingNeed(entityType, comp.housingNeed);

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
    PropertyInitializer::set<InGameResource>(comp.original,
                                             InGameResource(getResourceType(name), amount));
    comp.remainingAmount = comp.original.value().amount;

    return ComponentType(comp);
}

bool isInstanceOf(py::object module,
                  pybind11::handle entityDefinition,
                  const std::string& baseClass)
{
    if (py::hasattr(module, baseClass.c_str()))
    {
        py::object cls = module.attr(baseClass.c_str());
        if (py::isinstance(entityDefinition, cls))
        {
            return true;
        }
    }
    return false;
}

ComponentType EntityDefinitionLoader::createCompSelectible(uint32_t entityType,
                                                           py::object module,
                                                           pybind11::handle entityDefinition)
{
    if (isInstanceOf(module, entityDefinition, "Unit"))
    {
        auto iconHash = readIconDef(EntitySubTypes::UI_UNIT_ICON, entityDefinition);
        auto displayName = readValue<std::string>(entityDefinition, "display_name");

        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
        typeRegistry->registerHUDIcon(entityType, iconHash);
        typeRegistry->registerHUDDisplayName(entityType, displayName);

        CompSelectible comp;
        PropertyInitializer::set<GraphicsID>(comp.icon, iconHash);
        PropertyInitializer::set<std::string>(comp.displayName, displayName);
        return ComponentType(comp);
    }
    else if (isInstanceOf(module, entityDefinition, "Building"))
    {
        auto iconHash = readIconDef(EntitySubTypes::UI_BUILDING_ICON, entityDefinition);
        auto displayName = readValue<std::string>(entityDefinition, "display_name");

        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
        typeRegistry->registerHUDIcon(entityType, iconHash);
        typeRegistry->registerHUDDisplayName(entityType, displayName);

        CompSelectible comp;
        PropertyInitializer::set<GraphicsID>(comp.icon, iconHash);
        PropertyInitializer::set<std::string>(comp.displayName, displayName);
        return ComponentType(comp);
    }
    else if (isInstanceOf(module, entityDefinition, "NaturalResource"))
    {
        auto iconHash = readIconDef(EntitySubTypes::UI_NATURAL_RESOURCE_ICON, entityDefinition);
        auto displayName = readValue<std::string>(entityDefinition, "display_name");

        auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
        typeRegistry->registerHUDIcon(entityType, iconHash);
        typeRegistry->registerHUDDisplayName(entityType, displayName);

        CompSelectible comp;
        PropertyInitializer::set<GraphicsID>(comp.icon, iconHash);
        PropertyInitializer::set<std::string>(comp.displayName, displayName);
        comp.selectionIndicator = {
            GraphicAddon::Type::RHOMBUS,
            GraphicAddon::Rhombus{Constants::TILE_PIXEL_WIDTH, Constants::TILE_PIXEL_HEIGHT}};

        return ComponentType(comp);
    }
    return std::monostate{};
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

void EntityDefinitionLoader::setBoundingBoxReadFunc(
    std::function<core::Rect<int>(core::Ref<drs::DRSFile>, uint32_t)> func)
{
    m_boundingBoxReadFunc = func;
}

ComponentType EntityDefinitionLoader::createCompUnitFactory(pybind11::object module,
                                                            pybind11::handle entityDefinition)
{
    if (py::hasattr(module, "UnitFactory"))
    {
        py::object unitFactoryClass = module.attr("UnitFactory");
        if (py::isinstance(entityDefinition, unitFactoryClass))
        {
            CompUnitFactory comp;

            py::object producibleUnits = entityDefinition.attr("producible_units");
            std::vector<std::string> producibleUnitNames;
            std::unordered_map<char, std::string> entityNameByShortcut;

            for (auto unit : producibleUnits)
            {
                auto name = readValue<std::string>(unit, "name");
                auto shortcut = readValue<std::string>(unit, "shortcut");
                char key = 0;
                if (shortcut.empty() == false)
                {
                    key = shortcut.c_str()[0];
                }
                entityNameByShortcut[key] = name;
            }

            auto maxQueueSize = readValue<int>(entityDefinition, "max_queue_size");
            auto unitCreationSpeed = readValue<int>(entityDefinition, "unit_creation_speed");

            PropertyInitializer::set<uint32_t>(comp.maxQueueSize, maxQueueSize);
            PropertyInitializer::set<uint32_t>(comp.unitCreationSpeed, unitCreationSpeed);
            PropertyInitializer::set<std::vector<std::string>>(comp.producibleUnitNames,
                                                               producibleUnitNames);
            PropertyInitializer::set<std::unordered_map<char, std::string>>(
                comp.producibleUnitNamesByShortcuts, entityNameByShortcut);
            return ComponentType(comp);
        }
    }
    return std::monostate{};
}

ComponentType EntityDefinitionLoader::createCompHousing(pybind11::object module,
                                                        pybind11::handle entityDefinition)
{
    if (py::hasattr(module, "Housing"))
    {
        py::object housingClass = module.attr("Housing");
        if (py::isinstance(entityDefinition, housingClass))
        {
            CompHousing comp;
            auto housing = readValue<int>(entityDefinition, "housing_capacity");

            PropertyInitializer::set<uint32_t>(comp.housingCapacity, housing);
            return ComponentType(comp);
        }
    }
    return std::monostate{};
}

void EntityDefinitionLoader::loadEntityTypes(pybind11::object module)
{
    auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

    // TODO: Need to remove these hard codes along with stone and gold names
    // Specialized core entity types
    typeRegistry->registerEntityType("villager", EntityTypes::ET_VILLAGER);
    typeRegistry->registerEntityType("wood", EntityTypes::ET_TREE);
    typeRegistry->registerEntityType("construction_site", EntityTypes::ET_CONSTRUCTION_SITE);

    // Generic entity types
    if (py::hasattr(module, "all_entity_names"))
    {
        py::list allEntities = module.attr("all_entity_names");

        for (auto entry : allEntities)
        {
            auto name = entry.cast<std::string>();
            auto entityType = typeRegistry->getNextAvailableEntityType();

            typeRegistry->registerEntityType(name, entityType);
        }
    }
}

void EntityDefinitionLoader::validateEntities(pybind11::object module)
{
    py::object validateAllFunc = module.attr("validate_all");
    py::object result = validateAllFunc();
    bool success = result.cast<bool>();

    if (success == false)
    {
        throw std::runtime_error("Entity validation failed");
    }
    else
    {
        spdlog::debug("Entity validation passed");
    }
}

game::ComponentType EntityDefinitionLoader::createCompGarrison(pybind11::object module,
                                                               pybind11::handle entityDefinition)
{
    if (py::hasattr(module, "Garrison"))
    {
        py::object buildingClass = module.attr("Garrison");
        if (py::isinstance(entityDefinition, buildingClass))
        {
            CompGarrison comp;

            auto capacity = readValue<uint32_t>(entityDefinition, "garrison_capacity");
            auto unitTypesList =
                readValue<std::list<uint32_t>>(entityDefinition, "garrisonable_unit_types");

            std::unordered_set<UnitType> unitTypes;
            for (auto typeInt : unitTypesList)
            {
                unitTypes.insert(getUnitType(typeInt));
            }

            PropertyInitializer::set<uint32_t>(comp.capacity, capacity);
            PropertyInitializer::set<std::unordered_set<UnitType>>(comp.unitTypes, unitTypes);

            return ComponentType(comp);
        }
    }
    return std::monostate{};
}
