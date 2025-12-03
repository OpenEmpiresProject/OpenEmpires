#include "EntityLoader.h"

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
        else if (name == "gate")
            return EntitySubTypes::EST_GATE_SIZE;
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

BuildingOrientation getBuildingOrientation(const std::string& name)
{
    static unordered_map<string, BuildingOrientation> orientations = {
        {"right_angled", BuildingOrientation::RIGHT_ANGLED},
        {"left_angled", BuildingOrientation::LEFT_ANGLED},
        {"corner", BuildingOrientation::CORNER},
        {"horizontal", BuildingOrientation::HORIZONTAL},
        {"vertical", BuildingOrientation::VERTICAL},
        {"no_orientation", BuildingOrientation::NO_ORIENTATION},
    };
    return orientations.at(name);
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
    else if (name == "gate")
        return Size(4, 1);
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

EntityLoader::EntityLoader()
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

EntityLoader::~EntityLoader()
{
}

void EntityLoader::load()
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

void EntityLoader::loadUnits(py::object module)
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
                spdlog::error("Unknown entity {}, skipping loading. Check all_units.", name);
            }
        }
    }
}

void EntityLoader::loadNaturalResources(py::object module)
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
                    }
                }
            }
        }
    }
}

void EntityLoader::loadBuildings(pybind11::object module)
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
                attachConstructionSites(entityType, size);

                for (auto& drsIt : m_DRSDataByGraphicsIdHash)
                {
                    if (drsIt.first.entityType == entityType)
                    {
                        auto& drsData = drsIt.second;
                        auto& sc =
                            getComponent<CompSelectible>(entityType, EntitySubTypes::EST_DEFAULT);
                        sc.boundingBoxes[drsIt.first.direction] = drsData.boundingRect;
                    }
                }
            }
            else
            {
                spdlog::error("Unknown entity {}, skipping loading. Check all_buildings", name);
            }
        }
    }
}

void EntityLoader::loadConstructionSites(pybind11::object module)
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

void EntityLoader::loadTileSets(pybind11::object module)
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

void EntityLoader::loadUIElements(pybind11::object module)
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

EntityLoader::EntityDRSData EntityLoader::getDRSData(const GraphicsID& id)
{
    auto it = m_DRSDataByGraphicsIdHash.find(id);
    if (it != m_DRSDataByGraphicsIdHash.end())
        return it->second;
    else
    {
        spdlog::warn("Could not find DRS data for id {}. Following are existing mappings",
                     id.toString());
        for (auto& e : m_DRSDataByGraphicsIdHash)
        {
            spdlog::warn("DRS data mappings {} : SLP {}", e.first.toString(),
                         e.second.parts[0].slpId);
        }
    }
    return {};
}

uint32_t EntityLoader::createEntity(uint32_t entityType, uint32_t entitySubType)
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

void EntityLoader::addCommonComponents(py::object module,
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

void EntityLoader::addComponentsForUnit(uint32_t entityType)
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
    addComponentIfNotNull(entityType, CompPlayer());
    addComponentIfNotNull(entityType, graphics);
}

void EntityLoader::addComponentsForBuilding(uint32_t entityType)
{
    CompGraphics graphics;
    graphics.layer = GraphicLayer::ENTITIES;

    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompPlayer());
    addComponentIfNotNull(entityType, CompTransform());
    addComponentIfNotNull(entityType, graphics);
}

void EntityLoader::addComponentsForNaturalResource(uint32_t entityType)
{
    CompGraphics graphics;
    graphics.layer = GraphicLayer::ENTITIES;

    addComponentIfNotNull(entityType, graphics);
    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompTransform());
}

void EntityLoader::addComponentsForTileset(uint32_t entityType)
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
}

void EntityLoader::addComponentIfNotNull(uint32_t entityType, const ComponentType& comp)
{
    if (std::holds_alternative<std::monostate>(comp) == false)
        m_componentsByEntityType[entityType].push_back(comp);
}

void EntityLoader::loadDRSForAnimations(uint32_t entityType,
                                        uint32_t entitySubType,
                                        pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "animations"))
    {
        for (auto py_anim : entityDefinition.attr("animations"))
        {
            auto drsFileName = EntityLoader::readValue<std::string>(py_anim, "drs_file");
            auto slpId = EntityLoader::readValue<int>(py_anim, "slp_id");

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
            data.parts.push_back(EntityDRSData::Part(drsFile, slpId, Vec2::null, std::nullopt));
            data.boundingRect = boundingBox;

            m_DRSDataByGraphicsIdHash[id] = data;
        }
    }
}

void EntityLoader::processGraphic(uint32_t entityType,
                                  uint32_t entitySubType,
                                  py::handle graphicObj,
                                  const Vec2& anchor,
                                  BuildingOrientation orientation,
                                  bool flip,
                                  std::optional<int> frameIndex)
{
    // TODO: handle theme
    auto drsFileName = EntityLoader::readValue<std::string>(graphicObj, "drs_file");
    auto slpId = EntityLoader::readValue<int>(graphicObj, "slp_id");

    Vec2 anchorOverride = Vec2::null;
    if (py::hasattr(graphicObj, "anchor"))
    {
        py::object anchorObj = graphicObj.attr("anchor");
        anchorOverride.x = readValue<int>(anchorObj, "x");
        anchorOverride.y = readValue<int>(anchorObj, "y");
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
    id.orientation = (int) orientation;

    auto boundingBox = m_boundingBoxReadFunc(drsFile, slpId);

    if (m_DRSDataByGraphicsIdHash.contains(id))
    {
        m_DRSDataByGraphicsIdHash[id].parts.push_back(
            EntityDRSData::Part(drsFile, slpId, anchorOverride, frameIndex));
    }
    else
    {
        EntityDRSData data;
        data.parts.push_back(EntityDRSData::Part(drsFile, slpId, anchorOverride, frameIndex));
        data.boundingRect = boundingBox;
        data.clipRect = clipRect;
        data.anchor = anchor;
        data.flip = flip;

        m_DRSDataByGraphicsIdHash[id] = data;
    }

    if (frameIndex.has_value())
        spdlog::debug("Loaded DRS data for id {}, slp id {}:{}", id.toString(), slpId,
                      frameIndex.value());
    else
        spdlog::debug("Loaded DRS data for id {}, slp id {}", id.toString(), slpId);
}

void EntityLoader::loadDRSForStillImage(pybind11::object module,
                                        uint32_t entityType,
                                        uint32_t entitySubType,
                                        pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "graphics"))
    {
        py::object graphics = entityDefinition.attr("graphics");
        py::dict graphicByThemeDict = graphics.attr("by_theme");
        py::object singleGraphicClass = module.attr("SingleGraphic");
        py::object compositeGraphicClass = module.attr("CompositeGraphic");
        py::object orientatedGraphicClass = module.attr("OrientatedGraphic");

        for (auto [theme, graphicsEntry] : graphicByThemeDict)
        {
            if (py::isinstance(graphicsEntry, compositeGraphicClass))
            {
                bool flip = readValue<bool>(graphicsEntry, "flip");
                py::list parts = graphicsEntry.attr("parts");
                py::object anchor = graphicsEntry.attr("anchor");
                int anchorX = readValue<int>(anchor, "x");
                int anchorY = readValue<int>(anchor, "y");

                for (auto part : parts)
                    processGraphic(entityType, entitySubType, part, Vec2(anchorX, anchorY),
                                   BuildingOrientation::NO_ORIENTATION, flip, std::nullopt);
            }
            else if (py::isinstance(graphicsEntry, orientatedGraphicClass))
            {
                py::dict graphicsByOrientationDict = graphicsEntry.attr("by_orientation");

                for (auto [orientationEntry, graphic] : graphicsByOrientationDict)
                {
                    std::string orientationStr = py::str(orientationEntry);
                    BuildingOrientation orientation = getBuildingOrientation(orientationStr);

                    if (py::isinstance(graphic, singleGraphicClass))
                    {
                        bool flip = readValue<bool>(graphic, "flip");
                        std::optional<int> frameIndex;

                        if (py::hasattr(graphic, "frame_index"))
                        {
                            frameIndex = readValue<int>(graphic, "frame_index");
                        }

                        processGraphic(entityType, entitySubType, graphic, Vec2::null, orientation,
                                       flip, frameIndex);
                    }
                    else if (py::isinstance(graphic, compositeGraphicClass))
                    {
                        bool flip = readValue<bool>(graphic, "flip");
                        py::list parts = graphic.attr("parts");
                        py::object anchor = graphic.attr("anchor");
                        int anchorX = readValue<int>(anchor, "x");
                        int anchorY = readValue<int>(anchor, "y");

                        for (auto part : parts)
                            processGraphic(entityType, entitySubType, part, Vec2(anchorX, anchorY),
                                           orientation, flip, std::nullopt);
                    }
                    else
                    {
                        spdlog::error("Unknown graphic entry type for entity type {}", entityType);
                    }
                }
            }
            else
            {
                bool flip = readValue<bool>(graphicsEntry, "flip");
                processGraphic(entityType, entitySubType, graphicsEntry, Vec2::null,
                               BuildingOrientation::NO_ORIENTATION, flip, std::nullopt);
            }
        }
    }
}

void EntityLoader::loadDRSForIcon(uint32_t entityType,
                                  uint32_t entitySubType,
                                  pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "drs_file"))
    {
        auto drsFileName = EntityLoader::readValue<std::string>(entityDefinition, "drs_file");
        auto slpId = EntityLoader::readValue<int>(entityDefinition, "slp_id");

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
        data.parts.push_back(EntityDRSData::Part(drsFile, slpId, Vec2::null, std::nullopt));
        data.clipRect = clipRect;

        m_DRSDataByGraphicsIdHash[id] = data;
    }
}

EntityLoader::ConstructionSiteData EntityLoader::getSite(const std::string& sizeStr)
{
    return m_constructionSitesBySize.at(sizeStr);
}

void EntityLoader::setSite(const std::string& sizeStr, const std::map<int, int>& progressToFrames)
{
    m_constructionSitesBySize[sizeStr] = ConstructionSiteData{.progressToFrames = progressToFrames};
}

void EntityLoader::attachConstructionSites(uint32_t entityType, const std::string& sizeStr)
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

    for (const auto& [id, drs] : m_DRSDataByGraphicsIdHash)
    {
        if (id.entityType == entityType)
        {
            // Approach: Find the construction site DRS data and attach it to building entity
            // as entity sub types.
            GraphicsID siteId;
            siteId.entityType = EntityTypes::ET_CONSTRUCTION_SITE;
            siteId.entitySubType = getEntitySubType(EntityTypes::ET_CONSTRUCTION_SITE, sizeStr);
            siteId.orientation = id.orientation;

            // Try finding the construction site with exact orientation first, then without any
            // specific orientation. Eg: Both walls and gates have orientations, but only gate's
            // construction site has orientation specific graphics.
            //
            if (not m_DRSDataByGraphicsIdHash.contains(siteId))
            {
                siteId.orientation = (int) BuildingOrientation::NO_ORIENTATION;
                if (not m_DRSDataByGraphicsIdHash.contains(siteId))
                {
                    spdlog::error("Could not find construction site graphics for entity {}",
                                  id.toString());
                    continue;
                }
            }
            const auto& siteDRSData = m_DRSDataByGraphicsIdHash.at(siteId);

            GraphicsID buildingAttachedSiteId;
            buildingAttachedSiteId.entityType = entityType;
            buildingAttachedSiteId.entitySubType = siteId.entitySubType;
            buildingAttachedSiteId.orientation = id.orientation;
            m_DRSDataByGraphicsIdHash[buildingAttachedSiteId] = siteDRSData;
        }
    }
}

void EntityLoader::setDRSData(const GraphicsID& id, const EntityDRSData& data)
{
    m_DRSDataByGraphicsIdHash[id] = data;
}

void EntityLoader::setDRSLoaderFunc(std::function<Ref<DRSFile>(const std::string&)> func)
{
    m_drsLoadFunc = func;
}

GraphicsID EntityLoader::readIconDef(uint32_t entitySubType, pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "icon"))
    {
        py::object iconDef = entityDefinition.attr("icon");
        auto index = EntityLoader::readValue<int>(iconDef, "index");

        GraphicsID icon;
        icon.entityType = EntityTypes::ET_UI_ELEMENT;
        icon.entitySubType = entitySubType;
        icon.variation = index;

        loadDRSForIcon(EntityTypes::ET_UI_ELEMENT, entitySubType, iconDef);
        return icon;
    }
    return {};
}

ComponentType EntityLoader::createAnimation(py::object module, pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "animations") == false)
        return std::monostate{};

    CompAnimation comp;
    for (auto py_anim : entityDefinition.attr("animations"))
    {
        CompAnimation::ActionAnimation animation;
        animation.frames = EntityLoader::readValue<int>(py_anim, "frame_count");
        animation.speed = EntityLoader::readValue<int>(py_anim, "speed");
        animation.repeatable = EntityLoader::readValue<bool>(py_anim, "repeatable");
        auto name = EntityLoader::readValue<string>(py_anim, "name");
        auto action = getAction(name);

        PropertyInitializer::set(comp.animations[action], animation);
    }
    return ComponentType(comp);
}

ComponentType EntityLoader::createBuilder(py::object module, pybind11::handle entityDefinition)
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

ComponentType EntityLoader::createCompResourceGatherer(py::object module,
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

ComponentType EntityLoader::createCompUnit(uint32_t entityType,
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

ComponentType EntityLoader::createCompTransform(py::object module,
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

ComponentType EntityLoader::createCompResource(py::object module, pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "resource_amount") == false)
        return std::monostate{};

    auto name = EntityLoader::readValue<string>(entityDefinition, "name");
    auto amount = EntityLoader::readValue<int>(entityDefinition, "resource_amount");
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

ComponentType EntityLoader::createCompSelectible(uint32_t entityType,
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

ComponentType EntityLoader::createCompBuilding(py::object module, pybind11::handle entityDefinition)
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

            if (py::hasattr(entityDefinition, "default_orientation"))
            {
                std::string orientationStr =
                    readValue<std::string>(entityDefinition, "default_orientation");
                BuildingOrientation orientation = getBuildingOrientation(orientationStr);
                PropertyInitializer::set(comp.defaultOrientation, orientation);
            }

            if (py::hasattr(entityDefinition, "connected_constructions_allowed"))
            {
                auto seriesOfConstructions =
                    readValue<bool>(entityDefinition, "connected_constructions_allowed");
                PropertyInitializer::set(comp.connectedConstructionsAllowed, seriesOfConstructions);
            }

            return ComponentType(comp);
        }
    }
    return std::monostate{};
}

void EntityLoader::setBoundingBoxReadFunc(
    std::function<core::Rect<int>(core::Ref<drs::DRSFile>, uint32_t)> func)
{
    m_boundingBoxReadFunc = func;
}

ComponentType EntityLoader::createCompUnitFactory(pybind11::object module,
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

ComponentType EntityLoader::createCompHousing(pybind11::object module,
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

void EntityLoader::loadEntityTypes(pybind11::object module)
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

void EntityLoader::validateEntities(pybind11::object module)
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

game::ComponentType EntityLoader::createCompGarrison(pybind11::object module,
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
