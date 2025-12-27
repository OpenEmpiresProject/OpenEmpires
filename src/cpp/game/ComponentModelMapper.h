#ifndef GAME_COMPONENTMODELMAPPER_H
#define GAME_COMPONENTMODELMAPPER_H
#include "Property.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompResourceGatherer.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnitFactory.h"
#include "components/CompGarrison.h"
#include "components/CompVision.h"
#include "components/CompHousing.h"
#include "components/CompUnit.h"
#include "components/CompRendering.h"
#include "components/CompGraphics.h"

namespace game
{
template <auto Member> struct ModelPropertyMapping;

template <typename C, typename T, core::Property<T> C::*Member>
struct ModelPropertyMapping<Member>
{
    std::string_view propertyName{};
    std::string_view modelName;

    using component_type = C;
    using value_type = T;
    using property_type = core::Property<T>;

    static constexpr auto member = Member;

    constexpr ModelPropertyMapping(std::string_view model, std::string_view property)
        : propertyName(property), modelName(model)
    {
    }
};


template <typename C> 
struct ModelMapping
{
    std::string_view modelName;

    using component_type = C;

    constexpr ModelMapping(std::string_view model)
        : modelName(model)
    {
    }
};

template <typename M, typename = void> struct is_property_mapping : std::false_type
{
};

template <typename M>
struct is_property_mapping<M, std::void_t<decltype(M::member), typename M::value_type>>
    : std::true_type
{
};

template <typename M> inline constexpr bool is_property_mapping_v = is_property_mapping<M>::value;

class ComponentModelMapper
{
public:
    using ComponentType = std::variant<core::CompAction,
                                       core::CompBuilder,
                                       core::CompBuilding,
                                       core::CompEntityInfo,
                                       core::CompRendering,
                                       core::CompPlayer,
                                       core::CompResourceGatherer,
                                       core::CompResource,
                                       core::CompSelectible,
                                       core::CompTransform,
                                       core::CompUnitFactory,
                                       core::CompGarrison,
                                       core::CompVision,
                                       core::CompHousing,
                                       core::CompGraphics,
                                       core::CompAnimation,
                                       core::CompUnit>;

    // clang-format off
    static constexpr auto mappings()
    {
        return std::tuple{
            ModelPropertyMapping<&core::CompBuilder::buildSpeed>                    ("Builder", "build_speed"),
            ModelPropertyMapping<&core::CompBuilding::connectedConstructionsAllowed>("Building", "connected_constructions_allowed"),
            ModelPropertyMapping<&core::CompBuilding::defaultOrientationName>       ("Building", "default_orientation"),
            ModelPropertyMapping<&core::CompBuilding::sizeName>                     ("Building", "size"),
            ModelPropertyMapping<&core::CompBuilding::acceptedResourceNames>        ("ResourceDropOff", "accepted_resources"),
            ModelPropertyMapping<&core::CompEntityInfo::entityName>                 ("Model", "name"),
            ModelPropertyMapping<&core::CompResourceGatherer::capacity>             ("Gatherer", "resource_capacity"),
            ModelPropertyMapping<&core::CompResourceGatherer::gatherSpeed>          ("Gatherer", "gather_speed"),
            ModelPropertyMapping<&core::CompResource::resourceName>                 ("Model", "name"),
            ModelPropertyMapping<&core::CompResource::resourceAmount>               ("NaturalResource", "resource_amount"),
            ModelPropertyMapping<&core::CompSelectible::displayName>                ("Selectable", "display_name"),
            ModelPropertyMapping<&core::CompTransform::speed>                       ("Unit", "moving_speed"),
            ModelPropertyMapping<&core::CompUnitFactory::maxQueueSize>              ("UnitFactory", "max_queue_size"),
            ModelPropertyMapping<&core::CompUnitFactory::unitCreationSpeed>         ("UnitFactory", "unit_creation_speed"),
            ModelPropertyMapping<&core::CompGarrison::capacity>                     ("Garrison", "garrison_capacity"),
            ModelPropertyMapping<&core::CompGarrison::unitTypesInt>                 ("Garrison", "garrisonable_unit_types"),
            ModelPropertyMapping<&core::CompVision::lineOfSight>                    ("Vision", "line_of_sight"),
            ModelPropertyMapping<&core::CompVision::lineOfSightShapeStr>            ("Vision", "line_of_sight_shape"),
            ModelPropertyMapping<&core::CompVision::activeTracking>                 ("Vision", "active_tracking"),
            ModelPropertyMapping<&core::CompHousing::housingCapacity>               ("Housing", "housing_capacity"),
            ModelPropertyMapping<&core::CompUnit::housingNeed>                      ("Unit", "housing_need"),
            ModelPropertyMapping<&core::CompUnit::typeInt>                          ("Unit", "unit_type"),
            ModelMapping<core::CompTransform>                                       ("NaturalResource"),
            ModelMapping<core::CompTransform>                                       ("Building"),
            ModelMapping<core::CompTransform>                                       ("TileSet"),
            ModelMapping<core::CompAnimation>                                       ("Animated"),
            ModelMapping<core::CompAction>                                          ("Unit"),
            ModelMapping<core::CompRendering>                                       ("Model"),
            ModelMapping<core::CompPlayer>                                          ("Unit"),
            ModelMapping<core::CompPlayer>                                          ("Building"),
            ModelMapping<core::CompGraphics>                                        ("Model")
        };
    }
    // clang-format on
};
} // namespace game

#endif // GAME_COMPONENTMODELMAPPER_H
