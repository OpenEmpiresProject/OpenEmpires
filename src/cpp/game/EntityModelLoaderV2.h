#ifndef GAME_ENTITYMODELLOADERV2_H
#define GAME_ENTITYMODELLOADERV2_H

#include "EntityFactory.h"
#include "Property.h"
#include "GraphicsLoadupDataProvider.h"
#include "utils/Types.h"
#include "DRSFile.h"
#include <optional>
#include "Rect.h"
#include <variant>
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "components/CompUnitFactory.h"
#include "components/CompGarrison.h"
#include "components/CompHousing.h"

#include <pybind11/embed.h>

namespace game
{
struct DRSData : public core::GraphicsLoadupDataProvider::Data
{
    struct Part
    {
        core::Ref<drs::DRSFile> drsFile;
        int slpId = -1;
        core::Vec2 anchor = core::Vec2::null;
        std::optional<int> frameIndex;

        Part(core::Ref<drs::DRSFile> drs,
             int slp,
             const core::Vec2& anchor,
             std::optional<int> frameIndex);
    };
    std::vector<Part> parts;
    core::Rect<int> clipRect;
    core::Vec2 anchor;
    core::Rect<int> boundingRect;
    bool flip = false;
};


class EntityModelLoaderV2 : public core::EntityFactory, public core::PropertyInitializer, core::GraphicsLoadupDataProvider
{
public:
    EntityModelLoaderV2(const std::string& scriptDir);
    ~EntityModelLoaderV2();

    // TODO - evaluate to use EventHandler::init instead
    void init();

  private:
    void buildComponentModelMapping();

  private:
    uint32_t createEntity(uint32_t entityType, uint32_t entitySubType) override;
    Data getData(const core::GraphicsID& id) override;

    void loadAll(const pybind11::object& module);
    void loadEntityTypes(const pybind11::object& module);
    void postProcessing();

private:
    using ComponentType = std::variant<std::monostate,
                                       core::CompBuilder,
                                       core::CompBuilding,
                                       core::CompEntityInfo>;
  /*using ComponentType = std::variant<std::monostate,
                                     core::CompAction,
                                     core::CompAnimation,
                                     core::CompBuilder,
                                     core::CompBuilding,
                                     core::CompEntityInfo,
                                     core::CompGraphics,
                                     core::CompPlayer,
                                     core::CompRendering,
                                     core::CompResource,
                                     core::CompResourceGatherer,
                                     core::CompSelectible,
                                     core::CompTransform,
                                     core::CompUnit,
                                     core::CompUnitFactory,
                                     core::CompGarrison,
                                     core::CompVision,
                                     core::CompHousing>;*/

    struct ComponentHolder
    {
        std::list<ComponentType> components;
    };

    

    template <typename T> std::list<std::string> enrich(T& comp, const pybind11::object& obj)
    {
        std::list<std::string> processedFields;

        std::apply(
            [&](auto&&... props)
            {
                (
                    [&]
                    {
                        using Prop = std::decay_t<decltype(props)>;
                        using Value = typename Prop::value_type;
                        auto name = props.propertyName.data();

                        if (pybind11::hasattr(obj, name))
                        {
                            pybind11::object value = obj.attr(name);

                            PropertyInitializer::set<Value>(comp.*(Prop::member),
                                                            value.cast<Value>());

                            processedFields.push_back(std::string(name));
                        }
                    }(),
                    ...);
            },
            T::properties());

        return processedFields;
    }

    template <typename T>
    std::list<std::string> emplace(ComponentType& c, const pybind11::object& obj)
    {
        auto& comp = c.emplace<T>();
        return enrich(comp, obj);
    }

    using EmplaceFn = std::list<std::string> (EntityModelLoaderV2::*)(ComponentType&, const pybind11::object&);

    EmplaceFn getEmplaceFunc(std::type_index componentTypeIndex) const;

    std::map<uint32_t, core::Ref<ComponentHolder>> m_componentsByEntityType;
    std::map<std::string, core::Ref<ComponentHolder>> m_componentsByEntityName;
    const std::string m_scriptDir;

    struct UnprocessedFields
    {
        std::map<std::string, pybind11::object> fields;
    };
    std::map<std::string, UnprocessedFields> m_unprocessedFieldsByEntityName;
    void storeUnprocessedFields(const std::string& entityName,
                                const pybind11::object& obj,
                                const std::list<std::string>& processedFields);

    pybind11::object getUnprocessedField(const std::string& entityName,
                                         const std::string& fieldName);

    std::map<std::type_index, std::list<std::string_view>> m_modelsByComponentType;
    std::unordered_map<std::type_index, EmplaceFn> m_componentFactories;
};
} // namespace game

#endif // GAME_ENTITYMODELLOADERV2_H
