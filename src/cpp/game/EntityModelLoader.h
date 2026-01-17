#ifndef ENTITYDEFINITIONLOADER_H
#define ENTITYDEFINITIONLOADER_H

#include "DRSFile.h"
#include "EntityFactory.h"
#include "GraphicsRegistry.h"
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
#include "components/CompVision.h"
#include "utils/Size.h"
#include "utils/Types.h"

#include <iostream>
#include <list>
#include <map>
#include <optional>
#include <pybind11/embed.h>
#include <unordered_map>
#include <variant>

namespace game
{

using ComponentType = std::variant<std::monostate,
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
                                   core::CompHousing>;

class EntityModelLoader : public core::EntityFactory, public core::PropertyInitializer
{
  public:
    struct EntityDRSData
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
                 std::optional<int> frameIndex)
                : drsFile(drs), slpId(slp), anchor(anchor), frameIndex(frameIndex)
            {
            }
        };
        std::vector<Part> parts;
        core::Rect<int> clipRect;
        core::Vec2 anchor;
        core::Rect<int> boundingRect;
        bool flip = false;
    };
    EntityModelLoader();
    ~EntityModelLoader();

    void load();
    EntityDRSData getDRSData(const core::GraphicsID& id);

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

    static ComponentType createAnimation(pybind11::object module,
                                         pybind11::handle entityDefinition);
    static ComponentType createBuilder(pybind11::object module, pybind11::handle entityDefinition);
    static ComponentType createCompResourceGatherer(pybind11::object module,
                                                    pybind11::handle entityDefinition);
    static ComponentType createCompUnit(uint32_t entityType,
                                        pybind11::object module,
                                        pybind11::handle entityDefinition);
    static ComponentType createCompTransform(pybind11::object module,
                                             pybind11::handle entityDefinition);
    static ComponentType createCompResource(pybind11::object module,
                                            pybind11::handle entityDefinition);
    ComponentType createCompSelectible(uint32_t entityType,
                                       pybind11::object module,
                                       pybind11::handle entityDefinition);
    static ComponentType createCompBuilding(pybind11::object module,
                                            pybind11::handle entityDefinition);
    static ComponentType createCompVision(pybind11::object module,
                                          pybind11::handle entityDefinition);
    static ComponentType createCompGarrison(pybind11::object module,
                                            pybind11::handle entityDefinition);
    static ComponentType createCompUnitFactory(pybind11::object module,
                                               pybind11::handle entityDefinition);
    static ComponentType createCompHousing(pybind11::object module,
                                           pybind11::handle entityDefinition);
    uint32_t createEntity(uint32_t entityType) override;

  protected:
    struct ConstructionSiteData
    {
        core::Size size;
        std::map<int, int> progressToFrames;
    };

    void loadEntityTypes(pybind11::object module);
    void loadUnits(pybind11::object module);
    void loadNaturalResources(pybind11::object module);
    void loadBuildings(pybind11::object module);
    void loadTileSets(pybind11::object module);
    void loadUIElements(pybind11::object module);
    void addCommonComponents(pybind11::object module,
                             uint32_t entityType,
                             pybind11::handle entityDefinition);
    void addComponentsForUnit(uint32_t entityType);
    void addComponentsForBuilding(uint32_t entityType);
    void addComponentsForNaturalResource(uint32_t entityType);
    void addComponentsForTileset(uint32_t entityType);
    void addComponentIfNotNull(uint32_t entityType, const ComponentType& comp);
    void loadDRSForAnimations(uint32_t entityType, pybind11::handle entityDefinition);
    void loadDRSForStillImage(pybind11::object module,
                              uint32_t entityType,
                              pybind11::handle entityDefinition,
                              int uiElementType,
                              int isShadow);
    void loadDRSForIcon(uint32_t entityType,
                        uint32_t uiElementType,
                        pybind11::handle entityDefinition);
    core::GraphicsID readIconDef(uint32_t uiElementType, pybind11::handle entityDefinition);
    //ConstructionSiteData getSite(const std::string& sizeStr);
    void setSite(const std::string& sizeStr, const std::map<int, int>& progressToFrames);
    //void attachConstructionSites(uint32_t entityType, const std::string& sizeStr);
    void setDRSData(const core::GraphicsID& id, const EntityDRSData& data);
    void setDRSLoaderFunc(std::function<core::Ref<drs::DRSFile>(const std::string&)> func);
    void setBoundingBoxReadFunc(
        std::function<core::Rect<int>(core::Ref<drs::DRSFile>, uint32_t)> func);
    template <typename T> T& getComponent(uint32_t entityType)
    {
        auto mapKey = entityType;

        auto it = m_componentsByEntityType.find(mapKey);
        if (it != m_componentsByEntityType.end())
        {
            for (auto& variantComponent : it->second)
            {
                T* result = nullptr;

                std::visit(
                    [&](auto& comp)
                    {
                        using K = std::decay_t<decltype(comp)>;
                        if constexpr (std::is_same_v<K, T>)
                        {
                            result = &comp;
                        }
                    },
                    variantComponent);

                if (result)
                    return *result;
            }
        }
        throw std::runtime_error("Component of requested type not found");
    }

    void processGraphic(uint32_t entityType,
                        pybind11::handle graphicObj,
                        const core::Vec2& anchor,
                        core::BuildingOrientation orientation,
                        bool flip,
                        std::optional<int> frameIndex,
                        int state,
                        int uiElementType,
                        int isShadow,
                        bool isConstructionSite,
                        bool isIcon);

  private:
    const std::string m_unitsFile = "units";
    std::map<uint32_t, std::list<ComponentType>> m_componentsByEntityType;
    std::unordered_map<core::GraphicsID, EntityDRSData> m_DRSDataByGraphicsIdHash;
    std::unordered_map<std::string, core::Ref<drs::DRSFile>> m_drsFilesByName;
    std::function<core::Ref<drs::DRSFile>(const std::string&)> m_drsLoadFunc;
    std::function<core::Rect<int>(core::Ref<drs::DRSFile>, uint32_t)> m_boundingBoxReadFunc;

    const int m_entitySubTypeMapKeyOffset = 100000;
};

} // namespace game

#endif