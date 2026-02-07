#ifndef GAME_COMPONENTMODELMAPPER_H
#define GAME_COMPONENTMODELMAPPER_H
#include "Property.h"
#include "components/CompArmor.h"
#include "components/CompAttack.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompGarrison.h"
#include "components/CompGraphics.h"
#include "components/CompHealth.h"
#include "components/CompHousing.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUIElement.h"
#include "components/CompUnit.h"
#include "components/CompUnitFactory.h"
#include "components/CompVision.h"
#include "utils/Size.h"
#include "utils/Types.h"

#include <any>
#include <optional>

// TODO: move to a namespace
extern core::BuildingOrientation getBuildingOrientation(const std::string& name);
extern core::Size getBuildingSize(const std::string& name);
extern core::LineOfSightShape getLOSShape(const std::string& name);
extern int getUIElementType(const std::string& name);
extern std::unordered_map<char, uint32_t> getShortcuts(const std::any& value);
extern uint8_t getAcceptedResourceFlag(const std::any& value);
extern uint32_t getEntityType(const std::string& name);
extern std::unordered_set<core::UnitType> getGarrisonableUnitTypes(const std::any& value);
extern core::Ref<core::Command> getUnitDefaultCommand(const std::any&);
extern std::vector<int> getAttackOrArmorPerClass(const std::any&);
extern std::vector<float> getMultiplierPerClass(const std::any&);

namespace game
{

template <typename T> using ConverterFnStr = T (*)(const std::string&);
template <typename T> using ConverterFnInt = T (*)(int);
template <typename T> using ConverterFn = T (*)(const std::any&);

// DefaultStorage mechanism to store default values only for trivial types instead of
// putting the value in the ModelPropertyMapping because for non-trivial types ModelPropertyMapping
// construction will not be constexpr anymore.
//
template <typename T, bool = std::is_trivially_destructible_v<T> && std::is_trivially_copyable_v<T>>
struct DefaultStorage;

// trivial case ? store the value
template <typename T> struct DefaultStorage<T, true>
{
    // It is important to initialize this to comply with constexpr requirements
    T value{};
    bool isSet = false;
};

// non-trivial case ? store nothing
template <typename T> struct DefaultStorage<T, false>
{
    // empty
};

template <auto Member> struct ModelPropertyMapping;

template <typename C, typename T, core::Property<T> C::* Member>
struct ModelPropertyMapping<Member> : public DefaultStorage<T>
{
    std::string_view propertyName{};
    std::string_view modelName;
    ConverterFnStr<T> converterFuncStr = nullptr;
    ConverterFnInt<T> converterFuncInt = nullptr;
    ConverterFn<T> converterFunc = nullptr;

    using component_type = C;
    using value_type = T;
    using property_type = core::Property<T>;

    static constexpr auto member = Member;

    constexpr ModelPropertyMapping(std::string_view model, std::string_view property)
        : propertyName(property), modelName(model)
    {
    }

    constexpr ModelPropertyMapping(std::string_view model,
                                   std::string_view property,
                                   ConverterFnStr<T> converterFunc)
        : propertyName(property), modelName(model), converterFuncStr(converterFunc)
    {
    }

    constexpr ModelPropertyMapping(std::string_view model,
                                   std::string_view property,
                                   ConverterFnInt<T> converterFunc)
        : propertyName(property), modelName(model), converterFuncInt(converterFunc)
    {
    }

    constexpr ModelPropertyMapping(std::string_view model,
                                   std::string_view property,
                                   ConverterFn<T> converterFunc)
        : propertyName(property), modelName(model), converterFunc(converterFunc)
    {
    }

    constexpr ModelPropertyMapping(std::string_view model,
                                   std::string_view property,
                                   const T& defaultValue)
        : propertyName(property), modelName(model), DefaultStorage<T>{defaultValue, true}
    {
    }
};

template <typename C> struct ModelMapping
{
    std::string_view modelName;

    using component_type = C;

    constexpr ModelMapping(std::string_view model) : modelName(model)
    {
    }
};

class ComponentModelMapper
{
  public:
    using ComponentType = std::variant<core::CompAction,
                                       core::CompAnimation,
                                       core::CompArmor,
                                       core::CompAttack,
                                       core::CompBuilder,
                                       core::CompBuilding,
                                       core::CompEntityInfo,
                                       core::CompGarrison,
                                       core::CompGraphics,
                                       core::CompHealth,
                                       core::CompHousing,
                                       core::CompPlayer,
                                       core::CompResourceGatherer,
                                       core::CompRendering,
                                       core::CompResource,
                                       core::CompSelectible,
                                       core::CompTransform,
                                       core::CompUnitFactory,
                                       core::CompUIElement,
                                       core::CompUnit,
                                       core::CompVision>;

    // clang-format off
    static constexpr auto mappings()
    {
        return std::tuple{
            ModelPropertyMapping<&core::CompBuilder::buildSpeed>                    ("Builder", "build_speed"),
            ModelPropertyMapping<&core::CompBuilder::buildableTypesByShortcut>      ("Builder", "buildables", getShortcuts),
            ModelPropertyMapping<&core::CompBuilding::connectedConstructionsAllowed>("Building", "connected_constructions_allowed"),
            ModelPropertyMapping<&core::CompBuilding::defaultOrientation>           ("Building", "default_orientation", getBuildingOrientation),
            ModelPropertyMapping<&core::CompBuilding::size>                         ("Building", "size", getBuildingSize),
            ModelPropertyMapping<&core::CompBuilding::acceptedResourceNames>        ("ResourceDropOff", "accepted_resources"),
            ModelPropertyMapping<&core::CompBuilding::dropOffForResourceType>       ("ResourceDropOff", "accepted_resources", getAcceptedResourceFlag),
            ModelPropertyMapping<&core::CompBuilding::visualVariationByProgress>    ("Building", "progress_frame_map"),
            ModelPropertyMapping<&core::CompEntityInfo::entityName>                 ("Model", "name"),
            ModelPropertyMapping<&core::CompEntityInfo::entityType>                 ("Model", "name", getEntityType),
            ModelPropertyMapping<&core::CompUIElement::uiElementType>               ("UIElement", "element_name", getUIElementType),
            ModelPropertyMapping<&core::CompResourceGatherer::capacity>             ("Gatherer", "resource_capacity"),
            ModelPropertyMapping<&core::CompResourceGatherer::gatherSpeed>          ("Gatherer", "gather_speed"),
            ModelPropertyMapping<&core::CompResource::resourceName>                 ("NaturalResource", "name"),
            ModelPropertyMapping<&core::CompResource::resourceAmount>               ("NaturalResource", "resource_amount"),
            ModelPropertyMapping<&core::CompSelectible::displayName>                ("Selectable", "display_name"),
            ModelPropertyMapping<&core::CompTransform::speed>                       ("Unit", "moving_speed"),
            ModelPropertyMapping<&core::CompTransform::hasRotation>                 ("Unit", "moving_speed", true),
            ModelPropertyMapping<&core::CompUnitFactory::maxQueueSize>              ("UnitFactory", "max_queue_size"),
            ModelPropertyMapping<&core::CompUnitFactory::unitCreationSpeed>         ("UnitFactory", "unit_creation_speed"),
            ModelPropertyMapping<&core::CompUnitFactory::producibleUnitShortcuts>   ("UnitFactory", "producible_units", getShortcuts),
            ModelPropertyMapping<&core::CompGarrison::capacity>                     ("Garrison", "garrison_capacity"),
            ModelPropertyMapping<&core::CompGarrison::unitTypes>                    ("Garrison", "garrisonable_unit_types", getGarrisonableUnitTypes),
            ModelPropertyMapping<&core::CompVision::lineOfSight>                    ("Vision", "line_of_sight"),
            ModelPropertyMapping<&core::CompVision::lineOfSightShape>               ("Vision", "line_of_sight_shape", getLOSShape),
            ModelPropertyMapping<&core::CompVision::activeTracking>                 ("Vision", "active_tracking"),
            ModelPropertyMapping<&core::CompHousing::housingCapacity>               ("Housing", "housing_capacity"),
            ModelPropertyMapping<&core::CompUnit::housingNeed>                      ("Unit", "housing_need"),
            ModelPropertyMapping<&core::CompUnit::type>                             ("Unit", "unit_type", core::getUnitType),
            ModelPropertyMapping<&core::CompUnit::defaultCommand>                   ("Unit", "unit_type", getUnitDefaultCommand), // "unit_type" is a dummy here
            ModelPropertyMapping<&core::CompArmor::armorPerClass>                   ("Armor", "armor", getAttackOrArmorPerClass),
            ModelPropertyMapping<&core::CompArmor::damageResistance>                ("Armor", "damage_resistance"),
            ModelPropertyMapping<&core::CompAttack::attackPerClass>                 ("Attack", "attack", getAttackOrArmorPerClass),
            ModelPropertyMapping<&core::CompAttack::attackMultiplierPerClass>       ("Attack", "attack_multiplier", getMultiplierPerClass),
            ModelPropertyMapping<&core::CompHealth::maxHealth>                      ("Health", "health"),
            ModelMapping<core::CompTransform>                                       ("NaturalResource"),
            ModelMapping<core::CompTransform>                                       ("Building"),
            ModelMapping<core::CompTransform>                                       ("TileSet"),
            ModelMapping<core::CompAnimation>                                       ("Animated"),
            ModelMapping<core::CompAction>                                          ("Unit"),
            ModelMapping<core::CompRendering>                                       ("Model"),
            ModelMapping<core::CompPlayer>                                          ("Unit"),
            ModelMapping<core::CompPlayer>                                          ("Building"),
            ModelMapping<core::CompTransform>                                       ("UIElement"),
            ModelMapping<core::CompGraphics>                                        ("Model")
        };
    }
    // clang-format on
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

} // namespace game

#endif // GAME_COMPONENTMODELMAPPER_H
