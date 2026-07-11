#ifndef CORE_GIZMOS_H
#define CORE_GIZMOS_H
#include "Color.h"
#include "Feet.h"
#include "components/CompGraphics.h"

#include <entt/entity/registry.hpp>
#include <optional>
#include <set>
#include <variant>

namespace core
{
#ifdef DEBUG
class Gizmos
{
  public:
    Gizmos() = delete;
    ~Gizmos() = delete;

    static inline Color color = Color::BLACK;
    static inline uint32_t global = entt::null;

    /*
     *   Draw line from entity's position to end position. name should be unique to
     *   the entity. If Gizmos.global is used, then name should be universally unique.
     */
    static void drawLine(uint32_t entity,
                         const std::string& name,
                         const Feet& end,
                         std::optional<Color> color = std::nullopt);

    /*
     *   Draw line from given start position to end position. name should be unique to
     *   the entity. If Gizmos.global is used, then name should be universally unique.
     */
    static void drawLine(uint32_t entity,
                         const std::string& name,
                         const Feet& start,
                         const Feet& end,
                         std::optional<Color> color = std::nullopt);

    /*
     *  Draw line from entity's position towards direction with given length.
     *  Unlike above drawLine overloads, this version moves with the entity.
     *  name should be unique to the entity. If Gizmos.global is used, then
     *  name should be universally unique.
     */
    static void drawLine(uint32_t entity,
                         const std::string& name,
                         const Feet& direction,
                         int length,
                         std::optional<Color> color = std::nullopt);

    /*
     *   Draw line(s) connecting the points. name should be unique to
     *   the entity. If Gizmos.global is used, then name should be universally unique.
     */
    static void drawPath(uint32_t entity,
                         const std::string& name,
                         const std::list<Feet> points,
                         std::optional<Color> color = std::nullopt);

    /*
     *  Draw arrow from entity's position to end position. name should be unique to
     *  the entity. If Gizmos.global is used, then name should be universally unique.
     */
    static void drawArrow(uint32_t entity,
                          const std::string& name,
                          const Feet& end,
                          std::optional<Color> color = std::nullopt);

    /*
     *   Draw arrow from given start position to end position. name should be unique to
     *   the entity. If Gizmos.global is used, then name should be universally unique.
     */
    static void drawArrow(uint32_t entity,
                          const std::string& name,
                          const Feet& start,
                          const Feet& end,
                          std::optional<Color> color = std::nullopt);

    /*
     *  Draw arrow from entity's position towards direction with given length.
     *  This version moves with the entity. name should be unique to
     *   the entity. If Gizmos.global is used, then name should be universally unique.
     */
    static void drawArrow(uint32_t entity,
                          const std::string& name,
                          const Feet& direction,
                          int length,
                          std::optional<Color> color = std::nullopt);

    /*
     *  Draw isometric circle with radius centered to entity's position.
     *  This version moves with the entity. name should be unique to
     *   the entity. If Gizmos.global is used, then name should be universally unique.
     */
    static void drawCircle(uint32_t entity,
                           const std::string& name,
                           int radius,
                           std::optional<Color> color = std::nullopt,
                           bool filled = false);

    /*
     *  Draw isometric circle with radius around given center. name should be unique to
     *  the entity. If Gizmos.global is used, then name should be universally unique.
     */
    static void drawCircle(uint32_t entity,
                           const std::string& name,
                           const Feet& center,
                           int radius,
                           std::optional<Color> color = std::nullopt,
                           bool filled = false);

    /*
     *  Draw isometric circle with radius centered to entity's position but
     *  with offset with centerOffset pixel amount.
     *  This version moves with the entity as centerOffset represent only a
     *  relative offset. Absolute position will be derived from entity position
     *  + centerOffset.
     *  name should be unique to the entity. If Gizmos.global is used, then
     *  name should be universally unique.
     */
    static void drawCircle(uint32_t entity,
                           const std::string& name,
                           const Vec2& centerOffset,
                           int radius,
                           std::optional<Color> color = std::nullopt,
                           bool filled = false);

    /*
     *   Draw Quadrilateral according to the given corners.
     */
    static void drawQuad(uint32_t entity,
                         const std::string& name,
                         const std::array<Feet, 4> corners,
                         std::optional<Color> color = std::nullopt,
                         bool filled = false);

    /*
     *   Remove any existing drawing with the given name in the entity.
     */
    static void clearDrawing(uint32_t entity, const std::string& name);
    static void clearDrawing(uint32_t entity, CompGraphics& graphics, const std::string& name);

    static std::set<std::string>& getGizmoNames();
    static std::set<std::string> getGizmoGroupedNames();

  private:
    static std::set<std::string> s_gizmoNames;

    template <typename T> static T& getOrCreateGizmo(uint32_t entity, const std::string& name);
};
#endif

} // namespace core

#endif // CORE_GIZMOS_H
