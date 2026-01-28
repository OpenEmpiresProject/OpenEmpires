#include "EntityModelLoaderV2.h"

#include "ComponentModelMapper.h"
#include "EntityTypeRegistry.h"
#include "GameTypes.h"
#include "ServiceRegistry.h"
#include "utils/Size.h"

#include <filesystem>
#include <pybind11/stl.h>
#include <variant>
#include "utils/Utils.h"
#include "components/CompUIElement.h"
#include "commands/CmdIdle.h"

using namespace game;
using namespace core;
using namespace std;
using namespace drs;
namespace py = pybind11;

extern UnitAction getAction(const std::string actionname);
extern ResourceType getResourceType(const std::string& name);
extern BuildingOrientation getBuildingOrientation(const std::string& name);
extern int getState(const std::string& name);
extern Size getBuildingSize(const std::string& name);
extern LineOfSightShape getLOSShape(const std::string& name);
extern Rect<int> getBoundingBox(shared_ptr<DRSFile> drs, uint32_t slpId);
extern void validateEntities(pybind11::object module);
extern bool isInstanceOf(py::object module,
                         py::handle entityDefinition,
                         const std::string& baseClass);


// Performs default command initialization for unit component
// TODO : This shouldn't be in the game layer but in core layer
//void onUnitComponentCreate(entt::basic_registry<uint32_t>& reg, uint32_t e)
//{
//    auto& comp = reg.get<CompUnit>(e);
//
//    while (not comp.commandQueue.empty())
//    {
//        comp.commandQueue.pop();
//    }
//    auto cloned = comp.defaultCommand.value()->clone();
//    cloned->setEntityID(e);
//
//    comp.commandQueue.push(cloned);
//}



GraphicLayer getGraphicLayer(int layer)
{
    if (layer == (int)GraphicLayer::NONE)
    {
        return GraphicLayer::NONE;
    }
    else if (layer == (int)GraphicLayer::GROUND)
    {
        return GraphicLayer::GROUND;
    }
    else if (layer == (int)GraphicLayer::ON_GROUND)
    {
        return GraphicLayer::ON_GROUND;
    }
    else if (layer == (int)GraphicLayer::ENTITIES)
    {
        return GraphicLayer::ENTITIES;
    }
    else if (layer == (int)GraphicLayer::SKY)
    {
        return GraphicLayer::SKY;
    }
    else if (layer == (int)GraphicLayer::UI)
    {
        return GraphicLayer::UI;
    }
    else
    {
        throw std::runtime_error("Invalid graphic layer value: " + std::to_string(layer));
    }
}

void validatePython()
{
    py::module_ sys = py::module_::import("sys");

    auto paths = sys.attr("path");
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

template <typename T> static T readValue(pybind11::handle object, const std::string& key)
{
    if (pybind11::hasattr(object, key.c_str()))
    {
        return object.attr(key.c_str()).cast<T>();
    }
    else
    {
        auto cls = object.get_type();

        if (pybind11::hasattr(cls, key.c_str()))
            return cls.attr(key.c_str()).cast<T>();
    }
    return T();
}

template <typename Variant, typename F> struct VariantForEach;

template <typename... Ts, typename F> struct VariantForEach<std::variant<Ts...>, F>
{
    static void apply(F&& f)
    {
        (f.template operator()<Ts>(), ...);
    }
};

template <typename Variant, typename F> void for_each_variant_type(F&& f)
{
    VariantForEach<Variant, F>::apply(std::forward<F>(f));
}

template <typename Tuple, typename F> constexpr void for_each_tuple(Tuple&& t, F&& f)
{
    std::apply([&](auto&&... elems) { (f(elems), ...); }, std::forward<Tuple>(t));
}

struct _Constructible
{
    
};


struct SingleGraphic
{
    std::string drsFile;
    int slp_id;
    std::optional<Vec2> anchor;
    std::optional<int> frameIndex;
    std::optional<Rect<int>> clipRect;
    std::optional<bool> flip;
};

struct Icon : public SingleGraphic
{
    int index;
};


struct CompositeGraphic
{
    std::vector<SingleGraphic> parts;
    Vec2 anchor;
    std::optional<bool> flip;
};

using GraphicImpl = std::variant<SingleGraphic, CompositeGraphic>;

struct GraphicVariant
{
    GraphicImpl graphic;
    std::map<std::string, std::string> variationFilter;

    BuildingOrientation getBuildingOrientation() const
    {
        auto it = variationFilter.find("orientation");
        if (it != variationFilter.end())
        {
            return ::getBuildingOrientation(it->second);
        }
        return BuildingOrientation::NO_ORIENTATION;
    }

    int getState() const
    {
        auto it = variationFilter.find("state");
        if (it != variationFilter.end())
        {
            return ::getState(it->second);
        }
        return 0;
    }

    std::vector<SingleGraphic> getAllSingleGraphics() const
    {
        std::vector<SingleGraphic> result;
        std::visit(
            [&](auto&& graphic)
            {
                using T = std::decay_t<decltype(graphic)>;
                if constexpr (std::is_same_v<T, SingleGraphic>)
                {
                    result.push_back(graphic);
                }
                else if constexpr (std::is_same_v<T, CompositeGraphic>)
                {
                    result = graphic.parts;
                }
            },
            graphic);
        return result;
    }

    bool isComposite() const
    {
        return std::holds_alternative<CompositeGraphic>(graphic);
    }
};

struct Graphic
{
    std::vector<GraphicVariant> variants;
    GraphicLayer layer;
};

struct Animation
{
    std::string name;
    int frameCount;
    int speed;
    int slpId;
    std::string drsFile;
    bool repeatable;
};

struct Shortcut
{
    std::string name;
    std::string shortcut;
};

PYBIND11_EMBEDDED_MODULE(graphic_defs, m)
{
    py::class_<_Constructible>(m, "_Constructible")
        .def(py::init<>());

    py::class_<Vec2>(m, "Point")
        .def(py::init<int, int>(), py::arg("x"), py::arg("y"))
        .def_readonly("x", &Vec2::x)
        .def_readonly("y", &Vec2::y);

    py::class_<SingleGraphic>(m, "SingleGraphic")
        .def(py::init<std::string, int, std::optional<Vec2>, std::optional<int>,
                      std::optional<Rect<int>>, std::optional<bool>>(),
             py::kw_only(),
             py::arg("drs_file") = "graphics.drs",
             py::arg("slp_id"),
             py::arg("anchor") = std::nullopt, 
             py::arg("frame_index") = std::nullopt,
             py::arg("clip_rect") = std::nullopt,
             py::arg("flip") = std::nullopt)
        .def_readonly("drs_file", &SingleGraphic::drsFile)
        .def_readonly("slp_id", &SingleGraphic::slp_id)
        .def_readonly("frame_index", &SingleGraphic::frameIndex)
        .def_readonly("clip_rect", &SingleGraphic::clipRect)
        .def_readonly("flip", &SingleGraphic::flip)
        .def_readonly("anchor", &SingleGraphic::anchor);

    py::class_<Icon, SingleGraphic>(m, "Icon")
        .def(py::init<std::string, int, std::optional<Vec2>, int>(), py::kw_only(),
             py::arg("drs_file") = "graphics.drs", py::arg("slp_id"),
             py::arg("anchor") = std::nullopt, py::arg("index"))
        .def_readonly("index", &Icon::index);

    py::class_<CompositeGraphic>(m, "CompositeGraphic")
        .def(py::init<std::vector<SingleGraphic>, Vec2, std::optional<bool>>(), py::kw_only(),
             py::arg("parts"), py::arg("anchor"), py::arg("flip") = std::nullopt)
        .def_readonly("parts", &CompositeGraphic::parts)
        .def_readonly("flip", &CompositeGraphic::flip)
        .def_readonly("anchor", &CompositeGraphic::anchor);

    py::class_<GraphicVariant>(m, "GraphicVariant")
        .def(py::init<GraphicImpl, std::map<std::string, std::string>>(), py::kw_only(),
             py::arg("graphic"), py::arg("variation_filter"))
        .def_readonly("graphic", &GraphicVariant::graphic)
        .def_readonly("variation_filter", &GraphicVariant::variationFilter);

    py::class_<Graphic>(m, "Graphic")
        .def(py::init<std::vector<GraphicVariant>, GraphicLayer>(), py::kw_only(),
             py::arg("variants"),
             py::arg("layer"))
        .def_readonly("variants", &Graphic::variants)
        .def_readonly("layer", &Graphic::layer);

    py::class_<Animation>(m, "Animation")
        .def(py::init<std::string, int, int, int, std::string, bool>(),
             py::kw_only(), py::arg("name"), py::arg("frame_count") = 15,
             py::arg("speed") = 10, py::arg("slp_id"),
             py::arg("drs_file") = "graphics.drs", py::arg("repeatable") = true)
        .def_readonly("name", &Animation::name)
        .def_readonly("frame_count", &Animation::frameCount)
        .def_readonly("speed", &Animation::speed)
        .def_readonly("slp_id", &Animation::slpId)
        .def_readonly("drs_file", &Animation::drsFile)
        .def_readonly("repeatable", &Animation::repeatable);

    py::enum_<GraphicLayer>(m, "GraphicLayer")
        .value("NONE", GraphicLayer::NONE)
        .value("GROUND", GraphicLayer::GROUND)
        .value("ON_GROUND", GraphicLayer::ON_GROUND)
        .value("ENTITIES", GraphicLayer::ENTITIES)
        .value("SKY", GraphicLayer::SKY)
        .value("UI", GraphicLayer::UI)
        .export_values();

     py::class_<Shortcut>(m, "Shortcut")
        .def(py::init<std::string, std::string>(), py::kw_only(),
             py::arg("name"), py::arg("shortcut"))
        .def_readonly("name", &Shortcut::name)
        .def_readonly("shortcut", &Shortcut::shortcut);
}

DRSData getDRSData(const SingleGraphic& graphicData,
                   const CompositeGraphic& compositeData,
                   core::Ref<DRSInterface> drsInterface)
{
    auto drsFile = drsInterface->loadDRSFile(graphicData.drsFile);
    auto boundingBox = drsInterface->getBoundingBox(graphicData.drsFile,
                                                   graphicData.slp_id,
                                                    graphicData.frameIndex.value_or(0));

    DRSData data;
    data.parts.push_back(DRSData::Part(drsFile, graphicData.slp_id,
                                       graphicData.anchor.value_or(Vec2::null),
                                       graphicData.frameIndex,
                                       graphicData.flip.value_or(false)));
    data.boundingRect = boundingBox;
    data.clipRect = graphicData.clipRect.value_or(Rect<int>());
    data.anchor = compositeData.anchor;
    data.flip = compositeData.flip.value_or(false);

    return data;
}

DRSData getDRSData(const Animation& animation,
                   core::Ref<DRSInterface> drsInterface)
{
    auto drsFile = drsInterface->loadDRSFile(animation.drsFile);
    auto boundingBox = drsInterface->getBoundingBox(animation.drsFile, animation.slpId, 0);

    DRSData data;
    data.parts.push_back(DRSData::Part(drsFile, animation.slpId, Vec2::null, 0, false));
    data.boundingRect = boundingBox;

    return data;
}

DRSData::Part::Part(core::Ref<drs::DRSFile> drs,
                    int slp,
                    const core::Vec2& anchor,
                    std::optional<int> frameIndex, bool flip)
    : drsFile(drs), slpId(slp), anchor(anchor), frameIndex(frameIndex), flip(flip)
{
}

static const int g_entitySubTypeMapKeyOffset = 100000;

EntityModelLoaderV2::EntityModelLoaderV2(const std::string& scriptDir,
                                         core::Ref<DRSInterface> drsInterface)
    : m_scriptDir(scriptDir), m_drsInterface(drsInterface)
{
    // constructor
}

EntityModelLoaderV2::~EntityModelLoaderV2()
{
    // destructor
}

uint32_t EntityModelLoaderV2::createEntity(uint32_t entityType)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto entity = stateMan->createEntity();

    auto mapKey = entityType;

    auto it = m_componentsByEntityType.find(mapKey);
    if (it != m_componentsByEntityType.end())
    {
        auto& componentHolder = it->second;
        for (const auto& variantComponent : componentHolder->components)
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

    return entity;
}

const core::GraphicsLoadupDataProvider::Data& EntityModelLoaderV2::getData(
    const core::GraphicsID& id) const
{
    return m_DRSDataByGraphicsId.at(id);
}

bool EntityModelLoaderV2::hasData(const GraphicsID& id) const
{
    return m_DRSDataByGraphicsId.find(id) != m_DRSDataByGraphicsId.end();
}

void EntityModelLoaderV2::init()
{
    preprocessComponents();
    initPython();
    validatePython();
    auto module = loadModelImporterModule();
    validateEntities(module);
    loadEntityTypes(module);
    loadAll(module);
    loadUnprogressedFields();
    postProcessing();
}

void EntityModelLoaderV2::loadEntityTypes(const py::object& module)
{
    py::object pyNames = module.attr("all_entity_names");
    std::vector<std::string> entityNames = pyNames.cast<std::vector<std::string>>();

    auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

    // TODO: Need to remove these hard codes along with stone and gold names
    // Specialized core entity types
    typeRegistry->registerEntityType("villager", EntityTypes::ET_VILLAGER);
    typeRegistry->registerEntityType("wood", EntityTypes::ET_TREE);
    //typeRegistry->registerEntityType("construction_site", EntityTypes::ET_CONSTRUCTION_SITE);

    for (const auto& name : entityNames)
        typeRegistry->registerEntityType(name, typeRegistry->getNextAvailableEntityType());
}

/*
 *  Approach: Pass each model through all components types to see which component is interested
 *  in the model. And populate the component accordingly.
 */
void EntityModelLoaderV2::loadAll(const py::object& module)
{
    for (auto pyModel : module.attr("all_models"))
    {
        py::object pyObj = py::reinterpret_borrow<py::object>(pyModel);
        auto holder = core::CreateRef<ComponentHolder>();
        std::list<std::string> allProcessedFields;
        std::string entityName;

        for (auto [typeIndex, models] : m_modelsByComponentType)
        {
            for (auto modelName : models)
            {
                std::string modelNameStr = std::string(modelName);
                if (isInstanceOf(module, pyModel, modelNameStr))
                {
                    ComponentType component;

                    auto compFactory = getComponentFactoryFunc(typeIndex);
                    auto processedFields = (this->*compFactory)(component, pyObj);
                    allProcessedFields.splice(allProcessedFields.end(), processedFields);

                    holder->components.push_back(component);

                    if (std::holds_alternative<CompEntityInfo>(component))
                        entityName = std::get<CompEntityInfo>(component).entityName;
                }
            }
        }
        debug_assert(not entityName.empty(), "Entity info is not initialized");
        m_componentsByEntityName[entityName] = holder;

        storeUnprocessedFields(entityName, pyObj, allProcessedFields);
    }
}

void EntityModelLoaderV2::postProcessing()
{
    auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();

    for (auto [entityName, compHolder] : m_componentsByEntityName)
    {
        for (auto& componentVar : compHolder->components)
        {
            if (auto info = std::get_if<CompEntityInfo>(&componentVar))
            {
                uint32_t entityType = 0;
                // if (typeRegistry->isValid(entityName))
                entityType = typeRegistry->getEntityType(entityName);
                /*else
                {
                    entityType = typeRegistry->getNextAvailableEntityType();
                    typeRegistry->registerEntityType(entityName, entityType);
                }*/
                m_componentsByEntityType[entityType] = compHolder;
            }

            if (auto selectible = std::get_if<CompSelectible>(&componentVar))
            {
                auto id = compHolder->createGraphicsID();
                id.isConstructing = false;
                auto boundixBoxes = selectible->boundingBoxes.value();
                // TODO: Support multiple states
                boundixBoxes.resize(1, static_cast<int>(Direction::NONE) + 1); // 1 state

                // Approach: Try to find direction specific bounding boxes. Failing to find such
                // use the direction independent (i.e. Direction::NONE) bounding box by default
                //
                auto defaultBoundingBox = Rect<int>();
                id.direction = (int) Direction::NONE;
                if (hasData(id))
                {
                    DRSData& data = (DRSData&) getData(id);
                    defaultBoundingBox = data.boundingRect;
                }

                for (int i = static_cast<int>(Direction::NORTH);
                     i <= static_cast<int>(Direction::NONE); ++i)
                {
                    id.direction = i;
                    if (hasData(id))
                    {
                        DRSData& data = (DRSData&) getData(id);
                        boundixBoxes.set(0, i, data.boundingRect);
                    }
                    else
                    {
                        boundixBoxes.set(0, i, defaultBoundingBox);
                    }
                }
                PropertyInitializer::set(selectible->boundingBoxes, boundixBoxes);
            }

            if (auto unit = std::get_if<CompUnit>(&componentVar))
            {
                Ref<Command> idleCmd = CreateRef<CmdIdle>();
                idleCmd->setPriority(Command::DEFAULT_PRIORITY);
                PropertyInitializer::set(unit->defaultCommand, idleCmd);
            }

            if (auto resource = std::get_if<CompResource>(&componentVar))
            {
                PropertyInitializer::set<InGameResource>(resource->original,
                    InGameResource(getResourceType(resource->resourceName), resource->resourceAmount));
                resource->remainingAmount = resource->original.value().amount;
            }

            // Add component specific post processing (eg: lazy loading/enriching) here
        }
    }

 /*   auto& registry = stateMan->getRegistry();
    registry.on_construct<CompUnit>().connect<&onUnitComponentCreate>();*/
}

void EntityModelLoaderV2::storeUnprocessedFields(const std::string& entityName,
                                                 const pybind11::object& obj,
                                                 const std::list<std::string>& processedFields)
{
    py::dict dict = obj.attr("__dict__");
    UnprocessedFields unprocessed;

    for (auto item : dict)
    {
        std::string key = py::cast<std::string>(item.first);

        auto it = std::find(processedFields.begin(), processedFields.end(), key);

        if (it == processedFields.end())
        {
            py::object value = py::reinterpret_borrow<py::object>(item.second);
            unprocessed.fields[key] = value;
        }
    }

    if (not unprocessed.fields.empty())
        m_unprocessedFieldsByEntityName[entityName] = unprocessed;
}

pybind11::object EntityModelLoaderV2::getUnprocessedField(const std::string& entityName,
                                                          const std::string& fieldName)
{
    auto it = m_unprocessedFieldsByEntityName.find(entityName);

    if (it != m_unprocessedFieldsByEntityName.end())
    {
        auto fieldIt = it->second.fields.find(fieldName);

        if (fieldIt != it->second.fields.end())
        {
            return fieldIt->second;
        }
    }
    py::none();
}

void EntityModelLoaderV2::preprocessComponents()
{
    for_each_variant_type<ComponentType>(
        [&]<typename Comp>()
        {
            {
                constexpr auto mappings = ComponentModelMapper::mappings();

                m_modelsByComponentType[typeid(Comp)] = {};

                for_each_tuple(mappings,
                               [&]<typename M>(const M& mapping)
                               {
                                   if constexpr (std::is_same_v<typename M::component_type, Comp>)
                                   {
                                       m_modelsByComponentType[typeid(Comp)].insert(
                                           mapping.modelName);
                                   }
                               });

                m_componentFactories[typeid(Comp)] = &EntityModelLoaderV2::createAndEnrichComponent<Comp>;
            }
        });
}

EntityModelLoaderV2::ComponentFactoryFunc EntityModelLoaderV2::getComponentFactoryFunc(
    std::type_index componentTypeIndex) const
{
    auto it = m_componentFactories.find(componentTypeIndex);
    if (it != m_componentFactories.end())
        return it->second;

    return nullptr;
}

py::object EntityModelLoaderV2::loadModelImporterModule()
{
    return py::module_::import((m_scriptDir + ".model_importer").c_str());
}

void EntityModelLoaderV2::initPython()
{
    if (!Py_IsInitialized())
        py::initialize_interpreter();
}

void EntityModelLoaderV2::loadUnprogressedFields()
{
    auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

    for (auto [entityName, unprocessedFields] : m_unprocessedFieldsByEntityName)
    {
        auto& holder = m_componentsByEntityName[entityName];

        for (auto [fieldName, pyObj] : unprocessedFields.fields)
        {
            if (fieldName == "graphics")
            {
                auto graphic = pyObj.cast<Graphic>();
                auto compPtr = holder->tryGetComponent<CompGraphics>();
                PropertyInitializer::set(compPtr->layer, graphic.layer);
                
                for (const auto& variant : graphic.variants)
                {
                    auto allSingleGraphics = variant.getAllSingleGraphics();

                    auto compositeGraphic =
                        Utils::get_or<CompositeGraphic>(variant.graphic, CompositeGraphic());

                    for (const auto& singleGraphic : allSingleGraphics)
                    {
                        auto drsData = getDRSData(singleGraphic, compositeGraphic, m_drsInterface);
                        spam("Entity {} has graphic with DRS file {} and SLP id {}",
                             entityName, singleGraphic.drsFile, singleGraphic.slp_id);

                        auto graphicsId = holder->createGraphicsID(variant.variationFilter);
                        m_DRSDataByGraphicsId[graphicsId] = drsData;
                    }
                }
            }
            else if (fieldName == "animations")
            {
                auto compPtr = holder->tryGetComponent<CompAnimation>();
                debug_assert(compPtr, "Could not find animation component for entity {}", entityName);

                auto animations = pyObj.cast<std::list<Animation>>();

                for (const auto& animation : animations)
                {
                    auto drsData = getDRSData(animation, m_drsInterface);
                    spam("Entity {} has animation with DRS file {} and SLP id {} for action", entityName,
                         animation.drsFile, animation.slpId, animation.name);

                    auto entityType = typeRegistry->getEntityType(entityName);
                    // TODO - Support filters
                    GraphicsID graphicsId(entityType);
                    graphicsId.action = getAction(animation.name);

                    m_DRSDataByGraphicsId[graphicsId] = drsData;

                    CompAnimation::ActionAnimation actionAnimation;
                    actionAnimation.frames = animation.frameCount;
                    actionAnimation.speed = animation.speed;
                    actionAnimation.repeatable = animation.repeatable;
                    compPtr->animations[graphicsId.action] = actionAnimation;
                }
            }
        }
    }
}

 core::GraphicsID EntityModelLoaderV2::ComponentHolder::createGraphicsID(
    const std::map<std::string, std::string>& filters) const
 {
     auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

     GraphicsID id;

     if (filters.contains("orientation"))
         id.orientation = (int)getBuildingOrientation(filters.at("orientation"));
     if (filters.contains("state"))
         id.state = getState(filters.at("state"));
     if (filters.contains("construction_site"))
         id.isConstructing = filters.at("construction_site") == "true" ? 1 : 0;
     if (filters.contains("icon"))
         id.isIcon = filters.at("icon") == "true" ? 1 : 0;

     bool foundEntityType = false;

     for (auto& comp : components)
     {
         if (const auto& c = std::get_if<CompEntityInfo>(&comp))
         {
             id.entityType = typeRegistry->getEntityType(c->entityName);
             foundEntityType = true;
         }
         if (const auto& c = std::get_if<CompUIElement>(&comp))
         {
             id.uiElementType = c->uiElementType;
         }
 
         // id.action    --> Only needed for animated graphics
         // id.frame     --> Loaded in DRSGraphicsLoader
         // id.direction --> Loaded in DRSGraphicsLoader
         // id.playerId  --> Set at runtime
         // id.civilization  --> Not used atm
         // id.age           --> Not used atm
         // id.isShadow     --> TODO: Shadows need rework/redesign
     }
     debug_assert(foundEntityType, "Entity type is not found in component holder");
     return id;
 }

 core::GraphicsID EntityModelLoaderV2::ComponentHolder::createGraphicsID() const
 {
     GraphicsID id;

     for (auto& comp : components)
     {
         if (const auto& c = std::get_if<CompBuilding>(&comp))
         {
             id.orientation = (int) c->orientation;
             id.isConstructing = (int)c->isConstructing();
         }
         if (const auto& c = std::get_if<CompEntityInfo>(&comp))
         {
             id.entityType = c->entityType;
             id.state = c->state;
         }
         if (const auto& c = std::get_if<CompUIElement>(&comp))
             id.uiElementType = c->uiElementType;

         if (const auto& c = std::get_if<CompAction>(&comp))
             id.action = c->action;

         if (const auto& c = std::get_if<CompAnimation>(&comp))
             id.frame = c->frame;

         if (const auto& c = std::get_if<CompTransform>(&comp))
             id.direction = (int)c->getDirection();

         if (const auto& c = std::get_if<CompPlayer>(&comp))
         {
             if (c->player)
                 id.playerId = c->player->getId();
         }

         // TODO Support these
         // id.civilization  --> Not used atm
         // id.age           --> Not used atm
         // id.isShadow     --> TODO: Shadows need rework/redesign
     }
     return id;
 }

 std::unordered_map<char, uint32_t> getShortcuts(const std::any& value)
 {

     auto pyObj = std::any_cast<py::object>(value);
     auto shortcuts = pyObj.cast<std::list<Shortcut>>();

     std::unordered_map<char, uint32_t> entityTypesByShortcut;
     auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

     for (auto& shortcut : shortcuts)
     {
         if (typeRegistry->isValid(shortcut.name))
         {
             auto key = shortcut.shortcut.c_str()[0];
             entityTypesByShortcut[key] = typeRegistry->getEntityType(shortcut.name);
         }
     }
     return entityTypesByShortcut;
 }


uint8_t getAcceptedResourceFlag(const std::any& value)
{
    auto pyObj = std::any_cast<py::object>(value);
    auto acceptedResources = pyObj.cast<std::list<std::string>>();

    uint8_t acceptedResourceFlag = 0;
    for (auto& resStr : acceptedResources)
    {
        auto resType = getResourceType(resStr);
        acceptedResourceFlag |= resType;
    }
    return acceptedResourceFlag;
}

uint32_t getEntityType(const std::string& name)
{
    auto typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
    return typeRegistry->getEntityType(name);
}
