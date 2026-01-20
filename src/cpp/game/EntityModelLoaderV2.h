#ifndef GAME_ENTITYMODELLOADERV2_H
#define GAME_ENTITYMODELLOADERV2_H

#include "ComponentModelMapper.h"
#include "DRSFile.h"
#include "EntityFactory.h"
#include "GraphicsLoadupDataProvider.h"
#include "Property.h"
#include "Rect.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompGarrison.h"
#include "components/CompGraphics.h"
#include "components/CompHousing.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "components/CompUnitFactory.h"
#include "utils/Types.h"

#include <optional>
#include <pybind11/embed.h>
#include <variant>
#include "DRSInterface.h"

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
        bool flip = false;

        Part(core::Ref<drs::DRSFile> drs,
             int slp,
             const core::Vec2& anchor,
             std::optional<int> frameIndex,
             bool flip);
    };
    std::vector<Part> parts;
    core::Rect<int> clipRect;
    core::Vec2 anchor;
    core::Rect<int> boundingRect;
    bool flip = false;
};

class EntityModelLoaderV2 : public core::EntityFactory,
                            public core::PropertyInitializer,
                            public core::GraphicsLoadupDataProvider
{
  public:
    EntityModelLoaderV2(const std::string& scriptDir, core::Ref<DRSInterface> drsInterface);
    ~EntityModelLoaderV2();

    // TODO - evaluate to use EventHandler::init instead
    void init();

  private:
    void initPython();

  private:
    void preprocessComponents();

  private:
    uint32_t createEntity(uint32_t entityType) override;
    const Data& getData(const core::GraphicsID& id) override;

    void loadAll(const pybind11::object& module);
    void loadEntityTypes(const pybind11::object& module);
    void postProcessing();
    void loadUnprogressedFields();
    pybind11::object loadModelImporterModule();

  private:
    using ComponentType = ComponentModelMapper::ComponentType;
    struct ComponentHolder
    {
        std::list<ComponentType> components;

        core::GraphicsID createGraphicsID(const std::map<std::string, std::string>& filters) const;

        template<typename T> T* tryGetComponent()
        {
            for (auto& comp : components)
            {
                if (T* ptr = std::get_if<T>(&comp))
                    return ptr;
            }
            return nullptr;
        }
    };

    template <typename T> std::list<std::string> enrich(T& comp, const pybind11::object& obj)
    {
        std::list<std::string> processedFields;

        std::apply(
            [&](auto&&... mappings)
            {
                (
                    [&]
                    {
                        using M = std::decay_t<decltype(mappings)>;

                        // Handle only mappings that belong to this component T
                        if constexpr (std::is_same_v<typename M::component_type, T>)
                        {
                            // Only property mappings do enrichment. Component-Model mappings
                            // don't have any property to set. So those will be skipped here and
                            // created as an empty component in preprocessComponents().
                            //
                            if constexpr (is_property_mapping_v<M>)
                            {
                                const std::string_view name_sv = mappings.propertyName;
                                const std::string name{name_sv}; // ensure null-terminated

                                if (pybind11::hasattr(obj, name.c_str()))
                                {
                                    using Value = typename M::value_type;

                                    pybind11::object value = obj.attr(name.c_str());

                                    if (mappings.converterFuncStr != nullptr)
                                    {
                                        auto convertedValue =
                                            (*mappings.converterFuncStr)(value.cast<std::string>());
                                        PropertyInitializer::set<Value>(comp.*(M::member),
                                                                        convertedValue);
                                    }
                                    else if (mappings.converterFuncInt != nullptr)
                                    {
                                        auto convertedValue =
                                            (*mappings.converterFuncInt)(value.cast<int>());
                                        PropertyInitializer::set<Value>(comp.*(M::member),
                                                                        convertedValue);
                                    }
                                    else
                                    {
                                        PropertyInitializer::set<Value>(comp.*(M::member),
                                                                        value.cast<Value>());
                                    }

                                    processedFields.push_back(name);
                                }
                            }
                        }
                    }(),
                    ...);
            },
            ComponentModelMapper::mappings());

        return processedFields;
    }

    template <typename T>
    std::list<std::string> emplace(ComponentType& c, const pybind11::object& obj)
    {
        auto& comp = c.emplace<T>();
        return enrich(comp, obj);
    }

    using EmplaceFn = std::list<std::string> (EntityModelLoaderV2::*)(ComponentType&,
                                                                      const pybind11::object&);

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

    core::Ref<DRSInterface> m_drsInterface;

    std::unordered_map<core::GraphicsID, DRSData> m_DRSDataByGraphicsId;
};
} // namespace game

#endif // GAME_ENTITYMODELLOADERV2_H
