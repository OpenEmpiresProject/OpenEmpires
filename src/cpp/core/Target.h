#ifndef TARGET_H
#define TARGET_H
#include "Feet.h"

#include <optional>

namespace core
{

struct Target
{
    enum class Type
    {
        POSITION,
        BUILDING,
        RESOURCE,
        UNIT,
    };

    const Feet pos = Feet::null;
    const Type type = Type::POSITION;
    const std::optional<uint32_t> entity;

    Target() = default;

    Target(uint32_t entity) : entity(entity)
    {
    }

    Target(const Feet& pos, const Type type) : pos(pos), type(type)
    {
    }

    Target(const Feet& pos, const Type type, const uint32_t entity)
        : pos(pos), type(type), entity(entity)
    {
    }

    bool isValid() const
    {
        return not pos.isNull();
    }
};

} // namespace core

#endif