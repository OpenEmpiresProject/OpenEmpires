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

  private:
    uint32_t createEntity(uint32_t entityType) override;
    void createOrUpdateComponent(uint32_t entityType, pybind11::handle entityDefinition);
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