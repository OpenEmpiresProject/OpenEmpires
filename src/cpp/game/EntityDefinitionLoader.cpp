#include "EntityDefinitionLoader.h"

#include "GameTypes.h"
#include "debug.h"
#include "utils/Logger.h"

#include <unordered_map>

using namespace ion;
using namespace game;
using namespace std;
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

    debug_assert(actions.contains(actionname), "Unknown action {}", actionname);

    return actions[actionname];
}

uint32_t getEntityType(const std::string& entityName)
{
    static unordered_map<string, uint32_t> entityTypes = {
        {"villager", EntityTypes::ET_VILLAGER},
    };
    return entityTypes[entityName];
}

template <typename T> T readValue(pybind11::handle object, const std::string& key)
{
    if (py::hasattr(object, key.c_str()))
    {
        return object.attr(key.c_str()).cast<T>();
    }
    else
    {
        auto cls = object.get_type();

        if (py::hasattr(cls, key.c_str()))
            return cls.attr(key.c_str()).cast<T>();
    }
    return T();
}

ComponentType createAnimation(pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "animations") == false)
        return std::monostate{};

    CompAnimation comp;
    for (auto py_anim : entityDefinition.attr("animations"))
    {
        CompAnimation::ActionAnimation animation;
        animation.frames = readValue<int>(py_anim, "frame_count");
        animation.speed = readValue<int>(py_anim, "speed");
        animation.repeatable = readValue<bool>(py_anim, "repeatable");
        auto action = getAction(py_anim.attr("name").cast<std::string>());

        comp.animations[action] = animation;
    }
    return ComponentType(comp);
}

ComponentType creatBuilder(pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "build_speed") == false)
        return std::monostate{};

    auto buildSpeed = entityDefinition.attr("build_speed").cast<int>();
    return ComponentType(CompBuilder(buildSpeed));
}

ComponentType creatCompResourceGatherer(pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "gather_speed") == false)
        return std::monostate{};

    CompResourceGatherer comp;
    comp.gatherSpeed = entityDefinition.attr("gather_speed").cast<int>();
    comp.capacity = entityDefinition.attr("resource_capacity").cast<int>();
    return ComponentType(comp);
}

ComponentType creatCompUnit(pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "line_of_sight") == false)
        return std::monostate{};

    CompUnit comp;
    comp.lineOfSight = entityDefinition.attr("line_of_sight").cast<int>();
    return ComponentType(comp);
}

ComponentType creatCompTransform(pybind11::handle entityDefinition)
{
    if (py::hasattr(entityDefinition, "moving_speed") == false)
        return std::monostate{};

    CompTransform comp;
    comp.speed = entityDefinition.attr("moving_speed").cast<int>();
    comp.hasRotation = true;
    return ComponentType(comp);
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

    py::object units_module = py::module_::import("units");
    py::list all_units = units_module.attr("all_units");

    for (auto py_unit : all_units)
    {
        std::string name = py_unit.attr("name").cast<std::string>();

        auto entityType = getEntityType(name);
        addComponentsForUnit(entityType);
        createOrUpdateComponent(entityType, py_unit);
    }
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
    return entity;
}

void EntityDefinitionLoader::createOrUpdateComponent(uint32_t entityType,
                                                     pybind11::handle entityDefinition)
{
    py::dict fields = entityDefinition.attr("__dict__");

    addComponentIfNotNull(entityType, createAnimation(entityDefinition));
    addComponentIfNotNull(entityType, creatBuilder(entityDefinition));
    addComponentIfNotNull(entityType, creatCompResourceGatherer(entityDefinition));
    addComponentIfNotNull(entityType, creatCompUnit(entityDefinition));
    addComponentIfNotNull(entityType, creatCompTransform(entityDefinition));
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
