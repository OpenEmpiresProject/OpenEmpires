#ifndef CORE_GIZMODATA_H
#define CORE_GIZMODATA_H
#include "Color.h"
#include "Feet.h"

#include <entt/entity/registry.hpp>
#include <stdint.h>
#include <string>
#include <variant>

namespace core
{
struct RenderingContext;
class CompRendering;

struct GizmoData
{
    struct BaseGizmo
    {
        uint32_t entity = entt::null;
        std::string name;
        bool enabled = true;
        Color color;
    };

    struct LineGizmo : public BaseGizmo
    {
        Feet start = Feet::null;
        Feet end = Feet::null;
        Feet direction = Feet::null;
        int length = 0;

        void onRender(const RenderingContext& context, const CompRendering& comp);
    };

    struct ArrowGizmo : public LineGizmo
    {
        void onRender(const RenderingContext& context, const CompRendering& comp);
    };

    struct CircleGizmo : public BaseGizmo
    {
        Feet center = Feet::null;
        int radius = 0;
        Vec2 centerOffset = Vec2::null;
        bool filled = false;

        void onRender(const RenderingContext& context, const CompRendering& comp);
    };

    struct PathGizmo : public BaseGizmo
    {
        std::list<Feet> points;

        void onRender(const RenderingContext& context, const CompRendering& comp);
    };

    struct RhombusGizmo : public BaseGizmo
    {
        std::array<Feet, 4> corners;
        bool filled = true;

        void onRender(const RenderingContext& context, const CompRendering& comp);
    };

    using Data =
        std::variant<std::monostate, LineGizmo, ArrowGizmo, CircleGizmo, PathGizmo, RhombusGizmo>;

    Data data = std::monostate{};

    GizmoData()
    {
    }

    GizmoData(Data data) : data(data)
    {
    }

    template <typename T> const T& getGizmo() const
    {
        return std::get<T>(data);
    }

    template <typename T> T& getGizmo()
    {
        return std::get<T>(data);
    }

    template <typename T> T getGizmo() const
    {
        return std::get<T>(data);
    }

    BaseGizmo& getBaseGizmo()
    {
        return std::visit(
            [](auto& obj) -> BaseGizmo&
            {
                using T = std::decay_t<decltype(obj)>;

                if constexpr (std::is_same_v<T, std::monostate>)
                {
                    throw std::logic_error("Gizmo data not set");
                }
                else
                {
                    return obj;
                }
            },
            data);
    }
};
} // namespace core

#endif // CORE_GIZMODATA_H
