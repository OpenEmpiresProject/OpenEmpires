#include "Gizmos.h"

#include "GizmoData.h"
#include "StateManager.h"
#include "components/CompGraphics.h"

#include <stdexcept>

using namespace core;

std::set<std::string> Gizmos::s_gizmoNames;

template <typename T> T& core::Gizmos::getOrCreateGizmo(uint32_t entity, const std::string& name)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto& graphic = stateMan->getComponent<CompGraphics>(entity);

    auto [it, inserted] = graphic.gizmos.try_emplace(name, T());

    auto& data = it->second.getGizmo<T>();

    if (inserted)
    {
        data.entity = entity;
        data.name = name;
        s_gizmoNames.insert(name);
    }
    data.enabled = true;
    return data;
}

void Gizmos::drawLine(uint32_t entity,
                      const std::string& name,
                      const Feet& end,
                      std::optional<Color> color /*= std::nullopt*/)
{
    throw std::logic_error("This version of drawLine is not implemented");
}

void Gizmos::drawLine(uint32_t entity,
                      const std::string& name,
                      const Feet& start,
                      const Feet& end,
                      std::optional<Color> color /*= std::nullopt*/)
{
    throw std::logic_error("This version of drawLine is not implemented");
}

void Gizmos::drawLine(uint32_t entity,
                      const std::string& name,
                      const Feet& direction,
                      int length,
                      std::optional<Color> color /*= std::nullopt*/)
{
    throw std::logic_error("This version of drawLine is not implemented");
}

void Gizmos::drawPath(uint32_t entity,
                      const std::string& name,
                      const std::list<Feet> points,
                      std::optional<Color> color /*= std::nullopt*/)
{
    auto& gizmo = getOrCreateGizmo<GizmoData::PathGizmo>(entity, name);
    gizmo.color = color.value_or(Gizmos::color);
    gizmo.points = points;

    StateManager::markDirty(entity);
}

void Gizmos::drawArrow(uint32_t entity,
                       const std::string& name,
                       const Feet& end,
                       std::optional<Color> color /*= std::nullopt*/)
{
    throw std::logic_error("This version of drawArrow is not implemented");
}

void Gizmos::drawArrow(uint32_t entity,
                       const std::string& name,
                       const Feet& start,
                       const Feet& end,
                       std::optional<Color> color /*= std::nullopt*/)
{
    throw std::logic_error("This version of drawArrow is not implemented");
}

void Gizmos::drawArrow(uint32_t entity,
                       const std::string& name,
                       const Feet& direction,
                       int length,
                       std::optional<Color> color /*= std::nullopt*/)
{
    auto& gizmo = getOrCreateGizmo<GizmoData::ArrowGizmo>(entity, name);
    gizmo.color = color.value_or(Gizmos::color);
    gizmo.direction = direction;
    gizmo.length = length;

    StateManager::markDirty(entity);
}

void Gizmos::drawCircle(uint32_t entity,
                        const std::string& name,
                        int radius,
                        std::optional<Color> color /*= std::nullopt*/,
                        bool filled /*= false*/)
{
    auto& gizmo = getOrCreateGizmo<GizmoData::CircleGizmo>(entity, name);
    gizmo.color = color.value_or(Gizmos::color);
    gizmo.radius = radius;
    gizmo.filled = filled;

    StateManager::markDirty(entity);
}

void Gizmos::drawCircle(uint32_t entity,
                        const std::string& name,
                        const Feet& center,
                        int radius,
                        std::optional<Color> color /*= std::nullopt*/,
                        bool filled /*= false*/)
{
    auto& gizmo = getOrCreateGizmo<GizmoData::CircleGizmo>(entity, name);
    gizmo.color = color.value_or(Gizmos::color);
    gizmo.radius = radius;
    gizmo.filled = filled;
    gizmo.center = center;

    StateManager::markDirty(entity);
}

void Gizmos::drawCircle(uint32_t entity,
                        const std::string& name,
                        const Vec2& centerOffset,
                        int radius,
                        std::optional<Color> color /*= std::nullopt*/,
                        bool filled /*= false*/)
{
    throw std::logic_error("This version of drawCircle is not implemented");
}

void Gizmos::drawQuad(uint32_t entity,
                      const std::string& name,
                      const std::array<Feet, 4> corners,
                      std::optional<Color> color /*= std::nullopt*/,
                      bool filled /*= false*/)
{
    auto& gizmo = getOrCreateGizmo<GizmoData::RhombusGizmo>(entity, name);
    gizmo.color = color.value_or(Gizmos::color);
    gizmo.corners = corners;
    gizmo.filled = filled;

    StateManager::markDirty(entity);
}

void Gizmos::clearDrawing(uint32_t entity, const std::string& name)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto& graphic = stateMan->getComponent<CompGraphics>(entity);

    clearDrawing(entity, graphic, name);
}

void Gizmos::clearDrawing(uint32_t entity, CompGraphics& graphics, const std::string& name)
{
    auto stateMan = ServiceRegistry::getInstance().getService<StateManager>();

    auto it = graphics.gizmos.find(name);

    if (it != graphics.gizmos.end())
    {
        auto& base = it->second.getBaseGizmo();
        if (base.enabled)
        {
            base.enabled = false;
            StateManager::markDirty(entity);
        }
    }
}

std::set<std::string>& Gizmos::getGizmoNames()
{
    return s_gizmoNames;
}

std::set<std::string> Gizmos::getGizmoGroupedNames()
{
    std::set<std::string> groupedNames;

    for (auto& name : s_gizmoNames)
    {
        groupedNames.insert(name.substr(0, Constants::GIZMO_GROUPING_LENGTH));
    }
    return groupedNames;
}