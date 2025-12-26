#include "EntityModelLoader.h"

#include "EntityTypeRegistry.h"
#include "GameTypes.h"
#include "commands/CmdIdle.h"
#include "components/CompGarrison.h"
#include "components/CompHousing.h"
#include "components/CompUnitFactory.h"
#include "debug.h"
#include "logging/Logger.h"
#include "utils/ObjectPool.h"

#include <pybind11/stl.h>
#include <unordered_map>

using namespace core;
using namespace game;
using namespace std;
using namespace drs;
namespace py = pybind11;

void validateEntities(pybind11::object module)
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

int getUIElementType(const std::string& name)
{
    if (name == "resource_panel")
        return (int) UIElementTypes::RESOURCE_PANEL;
    else if (name == "control_panel")
        return (int) UIElementTypes::CONTROL_PANEL;
    else if (name == "progress_bar")
        return (int) UIElementTypes::PROGRESS_BAR;
    else if (name == "cursor")
        return (int) UIElementTypes::CURSOR;
    return (int) UIElementTypes::UNKNOWN;
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
        {"diagonal_forward", BuildingOrientation::DIAGONAL_FORWARD},
        {"diagonal_backward", BuildingOrientation::DIAGONAL_BACKWARD},
        {"corner", BuildingOrientation::CORNER},
        {"horizontal", BuildingOrientation::HORIZONTAL},
        {"vertical", BuildingOrientation::VERTICAL},
        {"no_orientation", BuildingOrientation::NO_ORIENTATION},
    };
    return orientations.at(name);
}

int getState(const std::string& name)
{
    static unordered_map<string, int> states = {{"closed", 0}, {"opened", 1}, {"stump", 1}};
    return states.at(name);
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

LineOfSightShape getLOSShape(const std::string& name)
{
    if (name == "circle")
        return LineOfSightShape::CIRCLE;
    else if (name == "rounded_square")
        return LineOfSightShape::ROUNDED_SQUARE;
    else
    {
        debug_assert(false, "unknown line-of-sight shape");
        return LineOfSightShape::UNKNOWN;
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

EntityModelLoader::EntityModelLoader()
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

EntityModelLoader::~EntityModelLoader()
{
}

void EntityModelLoader::load()
{
    py::scoped_interpreter guard{};

    py::module_ sys = py::module_::import("sys");
    sys.attr("path").attr("insert")(0, "./assets/scripts");

    validatePython(sys);

    py::object module = py::module_::import("model_importer");
    validateEntities(module);
    loadEntityTypes(module);
    loadUnits(module);
    loadNaturalResources(module);
    // loadConstructionSites(module);
    loadBuildings(module);
    loadTileSets(module);
    loadUIElements(module);
}

void EntityModelLoader::loadUnits(py::object module)
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

                // TODO: needs migration: start
                loadDRSForAnimations(entityType, py_unit);
                loadDRSForStillImage(module, entityType, py_unit, 0, 0);

                GraphicsID id;
                id.entityType = entityType;
                auto drsData = m_DRSDataByGraphicsIdHash.at(id);

                auto& sc = getComponent<CompSelectible>(entityType);

                auto boundingBoxes = sc.boundingBoxes.value();
                boundingBoxes.resize(1,
                                     static_cast<int>(Direction::NONE) + 1); // 1 state
                for (int i = static_cast<int>(Direction::NORTH);
                     i <= static_cast<int>(Direction::NORTHWEST); ++i)
                {
                    boundingBoxes.set(0, i, drsData.boundingRect);
                }
                PropertyInitializer::set(sc.boundingBoxes, boundingBoxes);

                // TODO: needs migration: end
            }
            else
            {
                spdlog::error("Unknown entity {}, skipping loading. Check all_units.", name);
            }
        }
    }
}

void EntityModelLoader::loadNaturalResources(py::object module)
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

                // TODO: needs migration: start
                loadDRSForStillImage(module, entityType, entry, 0, 0);

                GraphicsID id;
                id.entityType = entityType;
                auto drsData = m_DRSDataByGraphicsIdHash.at(id);

                auto& sc = getComponent<CompSelectible>(entityType);
                auto boundingBoxes = sc.boundingBoxes.value();
                boundingBoxes.resize(1,
                                     static_cast<int>(Direction::NONE) + 1); // 1 state
                boundingBoxes.set(0, static_cast<int>(Direction::NONE), drsData.boundingRect);
                PropertyInitializer::set(sc.boundingBoxes, boundingBoxes);

                py::object treeClass = module.attr("Tree");
                if (py::isinstance(entry, treeClass))
                {

                    if (py::hasattr(entry, "shadow"))
                    {
                        py::object stump = entry.attr("shadow");
                        loadDRSForStillImage(module, entityType, stump, 0, (int) true);

                        CompGraphics graphics;
                        PropertyInitializer::set(graphics.layer, GraphicLayer::ON_GROUND);

                        CompEntityInfo info(entityType);

                        auto entityTypeAndSubType = entityType;

                        addComponentIfNotNull(entityTypeAndSubType, graphics);
                        addComponentIfNotNull(entityTypeAndSubType, info);
                        addComponentIfNotNull(entityTypeAndSubType, CompRendering());
                        addComponentIfNotNull(entityTypeAndSubType, CompTransform());
                    }
                }
                // TODO: needs migration: end
            }
        }
    }
}

void EntityModelLoader::loadBuildings(pybind11::object module)
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

                // TODO: needs migration: start

                loadDRSForStillImage(module, entityType, entry, 0, 0);

                for (auto& drsIt : m_DRSDataByGraphicsIdHash)
                {
                    if (drsIt.first.entityType == entityType)
                    {
                        auto& drsData = drsIt.second;
                        auto& sc = getComponent<CompSelectible>(entityType);

                        auto boxesCopy = sc.boundingBoxes.value();
                        boxesCopy.resize(1,
                                         static_cast<int>(Direction::NONE) + 1); // 1 state
                        // Bounding box will be same regardless of the state
                        // TODO: Support different oriented buildings such as gates
                        boxesCopy.set(0, drsIt.first.direction,
                                      drsData.boundingRect); // only 1 state for buildings
                        PropertyInitializer::set(sc.boundingBoxes, boxesCopy);
                    }
                }
                // TODO: needs migration: end
            }
            else
            {
                spdlog::error("Unknown entity {}, skipping loading. Check all_buildings", name);
            }
        }
    }
}

void EntityModelLoader::loadTileSets(pybind11::object module)
{
    if (py::hasattr(module, "all_tilesets"))
    {
        py::list entries = module.attr("all_tilesets");

        for (auto entry : entries)
        {
            addComponentsForTileset(EntityTypes::ET_TILE);
            // TODO: needs migration: start
            loadDRSForStillImage(module, EntityTypes::ET_TILE, entry, 0, 0);
            // TODO: needs migration: end
        }
    }
}

void EntityModelLoader::loadUIElements(pybind11::object module)
{
    if (py::hasattr(module, "all_ui_elements"))
    {
        py::list entries = module.attr("all_ui_elements");

        for (auto entry : entries)
        {
            auto name = readValue<std::string>(entry, "name");

            auto elementType = getUIElementType(name);
            // TODO: needs migration: start
            loadDRSForStillImage(module, EntityTypes::ET_UI_ELEMENT, entry, elementType, 0);
            // TODO: needs migration: end
        }
    }
}

EntityModelLoader::EntityDRSData EntityModelLoader::getDRSData(const GraphicsID& id)
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

uint32_t EntityModelLoader::createEntity(uint32_t entityType)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto entity = stateMan->createEntity();

    auto mapKey = entityType;

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
                        spam("Adding component {} to entity {} of type {}", typeid(comp).name(),
                             entity, entityType);
                        stateMan->addComponent(entity, comp);
                    }
                },
                variantComponent);
        }
    }
    auto& info = stateMan->getComponent<CompEntityInfo>(entity);
    PropertyInitializer::set(info.entityId, entity);

    // TODO: needs migration: start

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
    // TODO: needs migration: end

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

void EntityModelLoader::addCommonComponents(py::object module,
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
    addComponentIfNotNull(entityType, createCompVision(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompUnitFactory(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompHousing(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompGarrison(module, entityDefinition));
    addComponentIfNotNull(entityType, createCompSelectible(entityType, module, entityDefinition));
    addComponentIfNotNull(entityType, CompEntityInfo(entityType));
}

void EntityModelLoader::addComponentsForUnit(uint32_t entityType)
{
    // TODO: needs migration: start

    CompGraphics graphics;
    PropertyInitializer::set(graphics.layer, GraphicLayer::ENTITIES);

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

    addComponentIfNotNull(entityType, CompAction(UnitAction::IDLE));

    // TODO: needs migration: end

    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompPlayer());
    addComponentIfNotNull(entityType, graphics);
}

void EntityModelLoader::addComponentsForBuilding(uint32_t entityType)
{
    // TODO: needs migration: start
    CompGraphics graphics;
    PropertyInitializer::set(graphics.layer, GraphicLayer::ENTITIES);

    // TODO: needs migration: end

    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompPlayer());
    addComponentIfNotNull(entityType, CompTransform());
    addComponentIfNotNull(entityType, graphics);
}

void EntityModelLoader::addComponentsForNaturalResource(uint32_t entityType)
{
    // TODO: needs migration: start
    CompGraphics graphics;
    PropertyInitializer::set(graphics.layer, GraphicLayer::ENTITIES);

    // TODO: needs migration: end

    addComponentIfNotNull(entityType, graphics);
    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompTransform());
}

void EntityModelLoader::addComponentsForTileset(uint32_t entityType)
{
    // TODO: needs migration: start

    CompGraphics graphics;
    PropertyInitializer::set(graphics.layer, GraphicLayer::GROUND);

    DebugOverlay overlay{DebugOverlay::Type::RHOMBUS, core::Color::GREY,
                         DebugOverlay::FixedPosition::BOTTOM_CENTER};
    overlay.customPos1 = DebugOverlay::FixedPosition::CENTER_LEFT;
    overlay.customPos2 = DebugOverlay::FixedPosition::CENTER_RIGHT;
    graphics.debugOverlays.push_back(overlay);
    // TODO: needs migration: end

    addComponentIfNotNull(entityType, graphics);
    addComponentIfNotNull(entityType, CompRendering());
    addComponentIfNotNull(entityType, CompTransform());
    addComponentIfNotNull(entityType, CompEntityInfo(entityType));
}

void EntityModelLoader::addComponentIfNotNull(uint32_t entityType, const ComponentType& comp)
{
    if (std::holds_alternative<std::monostate>(comp) == false)
        m_componentsByEntityType[entityType].push_back(comp);
}

void EntityModelLoader::loadDRSForAnimations(uint32_t entityType, pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "animations"))
    {
        for (auto py_anim : entityDefinition.attr("animations"))
        {
            auto drsFileName = EntityModelLoader::readValue<std::string>(py_anim, "drs_file");
            auto slpId = EntityModelLoader::readValue<int>(py_anim, "slp_id");

            Ref<DRSFile> drsFile;
            auto it = m_drsFilesByName.find(drsFileName);
            if (it != m_drsFilesByName.end())
                drsFile = it->second;
            else
                drsFile = m_drsLoadFunc("assets/" + drsFileName);

            auto action = getAction(py_anim.attr("name").cast<std::string>());

            GraphicsID id;
            id.entityType = entityType;
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

void EntityModelLoader::processGraphic(uint32_t entityType,
                                       py::handle graphicObj,
                                       const Vec2& anchor,
                                       BuildingOrientation orientation,
                                       bool flip,
                                       std::optional<int> frameIndex,
                                       int state,
                                       int uiElementType,
                                       int isShadow,
                                       bool isConstructionSite,
                                       bool isIcon)
{
    // TODO: handle theme
    auto drsFileName = EntityModelLoader::readValue<std::string>(graphicObj, "drs_file");
    auto slpId = EntityModelLoader::readValue<int>(graphicObj, "slp_id");

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
    id.orientation = (int) orientation;
    id.state = state;
    id.uiElementType = uiElementType;
    id.isShadow = isShadow;
    id.isConstructing = isConstructionSite;
    id.isIcon = isIcon;

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

void EntityModelLoader::loadDRSForStillImage(pybind11::object module,
                                             uint32_t entityType,
                                             pybind11::handle entityDefinition,
                                             int uiElementType,
                                             int isShadow)
{
    if (py::hasattr(entityDefinition, "graphics"))
    {
        py::object graphics = entityDefinition.attr("graphics");
        py::list graphicVariants = graphics.attr("variants");
        py::object singleGraphicClass = module.attr("SingleGraphic");
        py::object compositeGraphicClass = module.attr("CompositeGraphic");

        for (auto graphicsVariant : graphicVariants)
        {
            py::object graphicsEntry = graphicsVariant.attr("graphic");
            py::dict variationFilters = graphicsVariant.attr("variation_filter");

            std::map<std::string, std::string> filters;
            for (auto [key, value] : variationFilters)
            {
                auto keyStr = py::cast<std::string>(key);
                auto valueStr = py::cast<std::string>(value);
                filters.insert({keyStr, valueStr});
            }

            BuildingOrientation orientation = BuildingOrientation::NO_ORIENTATION;
            if (filters.contains("orientation"))
            {
                std::string orientationStr = filters.find("orientation")->second;
                orientation = getBuildingOrientation(orientationStr);
            }

            int state = 0;
            if (filters.contains("state"))
            {
                std::string stateStr = filters.find("state")->second;
                state = getState(stateStr);
            }

            bool isConstructionSite = filters.contains("construction_site");
            bool isIcon = filters.contains("icon");

            if (py::isinstance(graphicsEntry, compositeGraphicClass))
            {
                bool flip = readValue<bool>(graphicsEntry, "flip");
                py::list parts = graphicsEntry.attr("parts");
                py::object anchor = graphicsEntry.attr("anchor");
                int anchorX = readValue<int>(anchor, "x");
                int anchorY = readValue<int>(anchor, "y");

                for (auto part : parts)
                    processGraphic(entityType, part, Vec2(anchorX, anchorY), orientation, flip,
                                   std::nullopt, state, uiElementType, isShadow, isConstructionSite,
                                   isIcon);
            }
            else
            {
                std::optional<int> frameIndex;
                if (py::hasattr(graphicsEntry, "frame_index"))
                {
                    frameIndex = readValue<int>(graphicsEntry, "frame_index");
                }
                bool flip = readValue<bool>(graphicsEntry, "flip");
                processGraphic(entityType, graphicsEntry, Vec2::null, orientation, flip, frameIndex,
                               state, uiElementType, isShadow, isConstructionSite, isIcon);
            }
        }
    }
}

void EntityModelLoader::loadDRSForIcon(uint32_t entityType,
                                       uint32_t uiElementType,
                                       pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "drs_file"))
    {
        auto drsFileName = EntityModelLoader::readValue<std::string>(entityDefinition, "drs_file");
        auto slpId = EntityModelLoader::readValue<int>(entityDefinition, "slp_id");

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
        id.uiElementType = uiElementType;

        EntityDRSData data;
        data.parts.push_back(EntityDRSData::Part(drsFile, slpId, Vec2::null, std::nullopt));
        data.clipRect = clipRect;

        m_DRSDataByGraphicsIdHash[id] = data;
    }
}

void EntityModelLoader::setDRSData(const GraphicsID& id, const EntityDRSData& data)
{
    m_DRSDataByGraphicsIdHash[id] = data;
}

void EntityModelLoader::setDRSLoaderFunc(std::function<Ref<DRSFile>(const std::string&)> func)
{
    m_drsLoadFunc = func;
}

GraphicsID EntityModelLoader::readIconDef(uint32_t uiElementType, pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "icon"))
    {
        py::object iconDef = entityDefinition.attr("icon");
        auto index = EntityModelLoader::readValue<int>(iconDef, "index");

        GraphicsID icon;
        icon.entityType = EntityTypes::ET_UI_ELEMENT;
        icon.uiElementType = uiElementType;
        icon.variation = index;
        icon.isIcon = (int) true;

        loadDRSForIcon(EntityTypes::ET_UI_ELEMENT, uiElementType, iconDef);
        return icon;
    }
    return {};
}

ComponentType EntityModelLoader::createAnimation(py::object module,
                                                 pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "animations") == false)
        return std::monostate{};

    CompAnimation comp;
    for (auto py_anim : entityDefinition.attr("animations"))
    {
        CompAnimation::ActionAnimation animation;
        animation.frames = EntityModelLoader::readValue<int>(py_anim, "frame_count");
        animation.speed = EntityModelLoader::readValue<int>(py_anim, "speed");
        animation.repeatable = EntityModelLoader::readValue<bool>(py_anim, "repeatable");
        auto name = EntityModelLoader::readValue<string>(py_anim, "name");
        auto action = getAction(name);

        PropertyInitializer::set(comp.animations[action], animation);
    }
    return ComponentType(comp);
}

ComponentType EntityModelLoader::createBuilder(py::object module, pybind11::handle entityDefinition)
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

ComponentType EntityModelLoader::createCompResourceGatherer(py::object module,
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

ComponentType EntityModelLoader::createCompUnit(uint32_t entityType,
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

ComponentType EntityModelLoader::createCompTransform(py::object module,
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

ComponentType EntityModelLoader::createCompResource(py::object module,
                                                    pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "resource_amount") == false)
        return std::monostate{};

    auto name = EntityModelLoader::readValue<string>(entityDefinition, "name");
    auto amount = EntityModelLoader::readValue<int>(entityDefinition, "resource_amount");
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

ComponentType EntityModelLoader::createCompSelectible(uint32_t entityType,
                                                      py::object module,
                                                      pybind11::handle entityDefinition)
{
    if (isInstanceOf(module, entityDefinition, "Unit"))
    {
        auto iconHash = readIconDef((int) UIElementTypes::UNIT_ICON, entityDefinition);
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
        auto iconHash = readIconDef((int) UIElementTypes::BUILDING_ICON, entityDefinition);
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
        auto iconHash = readIconDef((int) UIElementTypes::NATURAL_RESOURCE_ICON, entityDefinition);
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

ComponentType EntityModelLoader::createCompBuilding(py::object module,
                                                    pybind11::handle entityDefinition)
{
    if (py::hasattr(module, "Building"))
    {
        py::object buildingClass = module.attr("Building");
        if (py::isinstance(entityDefinition, buildingClass))
        {
            CompBuilding comp;

            auto sizeStr = readValue<std::string>(entityDefinition, "size");
            // auto constructionSiteName = readValue<std::string>(entityDefinition,
            // "construction_site");
            auto size = getBuildingSize(sizeStr);

            std::map<int, int> progressMap;
            try
            {
                progressMap = readValue<std::map<int, int>>(entityDefinition, "progress_frame_map");
            }
            catch (const std::exception& e)
            {
                auto what = e.what();
                int ii = 0;
            }

            PropertyInitializer::set<Size>(comp.size, size);
            // PropertyInitializer::set<std::string>(comp.constructionSiteName,
            // constructionSiteName);
            PropertyInitializer::set<std::map<int, int>>(comp.visualVariationByProgress,
                                                         progressMap);

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

void EntityModelLoader::setBoundingBoxReadFunc(
    std::function<core::Rect<int>(core::Ref<drs::DRSFile>, uint32_t)> func)
{
    m_boundingBoxReadFunc = func;
}

ComponentType EntityModelLoader::createCompUnitFactory(pybind11::object module,
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

ComponentType EntityModelLoader::createCompHousing(pybind11::object module,
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

void EntityModelLoader::loadEntityTypes(pybind11::object module)
{
    auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

    // TODO: Need to remove these hard codes along with stone and gold names
    // Specialized core entity types
    typeRegistry->registerEntityType("villager", EntityTypes::ET_VILLAGER);
    typeRegistry->registerEntityType("wood", EntityTypes::ET_TREE);
    // typeRegistry->registerEntityType("construction_site", EntityTypes::ET_CONSTRUCTION_SITE);

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

game::ComponentType EntityModelLoader::createCompGarrison(pybind11::object module,
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

ComponentType EntityModelLoader::createCompVision(pybind11::object module,
                                                  pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "line_of_sight"))
    {
        int los = entityDefinition.attr("line_of_sight").cast<int>();
        bool track = entityDefinition.attr("active_tracking").cast<bool>();
        std::string losShapeStr = entityDefinition.attr("line_of_sight_shape").cast<std::string>();
        LineOfSightShape losShape = getLOSShape(losShapeStr);

        CompVision comp;
        PropertyInitializer::set<uint32_t>(comp.lineOfSight, los);
        PropertyInitializer::set<bool>(comp.activeTracking, track);
        PropertyInitializer::set<LineOfSightShape>(comp.lineOfSightShape, losShape);
        return ComponentType(comp);
    }
    return std::monostate{};
}
