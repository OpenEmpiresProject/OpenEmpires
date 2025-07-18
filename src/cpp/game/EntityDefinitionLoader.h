#ifndef ENTITYDEFINITIONLOADER_H
#define ENTITYDEFINITIONLOADER_H

#include "EntityFactory.h"
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

#include <pybind11/embed.h>
#include <iostream>
#include <variant>
#include <map>
#include <list>


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
        EntityDefinitionLoader(/* args */);
        ~EntityDefinitionLoader();

        void load();

    private:
        uint32_t createEntity(uint32_t entityType) override;
        void createOrUpdateComponent(uint32_t entityType, pybind11::handle entityDefinition);
        void addComponentsForUnit(uint32_t entityType);
        void addComponentIfNotNull(uint32_t entityType, const ComponentType& comp);

    private:
        const std::string m_unitsFile = "units";
        std::map<uint32_t, std::list<ComponentType>> m_componentsByEntityType;
    };
  
    
} // namespace ion


#endif