#ifndef ENTITYDEFINITIONLOADER_H
#define ENTITYDEFINITIONLOADER_H

#include "DRSFile.h"
#include "EntityFactory.h"
#include "GraphicsRegistry.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/Types.h"

#include <iostream>
#include <list>
#include <map>
#include <pybind11/embed.h>
#include <unordered_map>
#include <variant>

namespace game
{

using ComponentType = std::variant<std::monostate,
                                   ion::CompAction,
                                   ion::CompAnimation,
                                   ion::CompBuilder,
                                   ion::CompBuilding,
                                   ion::CompDirty,
                                   ion::CompEntityInfo,
                                   ion::CompGraphics,
                                   ion::CompPlayer,
                                   ion::CompRendering,
                                   ion::CompResource,
                                   ion::CompResourceGatherer,
                                   ion::CompSelectible,
                                   ion::CompTransform,
                                   ion::CompUnit>;

class EntityDefinitionLoader : public ion::EntityFactory
{
  public:
    struct EntityDRSData
    {
        ion::Ref<drs::DRSFile> drsFile;
        int slpId = -1;
    };
    EntityDefinitionLoader(/* args */);
    ~EntityDefinitionLoader();

    void load();
    EntityDRSData getDRSData(const ion::GraphicsID& id);

    template <typename T> 
    static T readValue(pybind11::handle object, const std::string& key)
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
    static ComponentType createCompUnit(pybind11::object module, pybind11::handle entityDefinition);
    static ComponentType createCompTransform(pybind11::object module,
                                             pybind11::handle entityDefinition);
    static ComponentType createCompResource(pybind11::object module,
                                            pybind11::handle entityDefinition);
    static ComponentType createCompBuilding(pybind11::object module,
                                            pybind11::handle entityDefinition);


  protected:
    uint32_t createEntity(uint32_t entityType) override;
    void loadUnits(pybind11::object module);
    void loadNaturalResources(pybind11::object module);
    void loadBuildings(pybind11::object module);
    void createOrUpdateComponent(pybind11::object module,
                                 uint32_t entityType,
                                 pybind11::handle entityDefinition);
    void addComponentsForUnit(uint32_t entityType);
    void addComponentIfNotNull(uint32_t entityType, const ComponentType& comp);
    void updateDRSData(uint32_t entityType, pybind11::handle entityDefinition);

  private:
    const std::string m_unitsFile = "units";
    std::map<uint32_t, std::list<ComponentType>> m_componentsByEntityType;
    std::unordered_map<int64_t, EntityDRSData> m_DRSDataByGraphicsIdHash;
    std::unordered_map<std::string, ion::Ref<drs::DRSFile>> m_drsFilesByName;
};

} // namespace game

#endif