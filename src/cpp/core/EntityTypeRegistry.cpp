#include "EntityTypeRegistry.h"

#include "utils/Logger.h"

using namespace core;

bool EntityTypeRegistry::isValid(uint32_t entityTye) const
{
    return m_entityTypes.contains(entityTye);
}

bool EntityTypeRegistry::isValid(const std::string& name) const
{
    return m_entityTypesByNames.contains(name);
}

uint32_t EntityTypeRegistry::getEntityType(const std::string& name) const
{
    return m_entityTypesByNames.at(name);
}

void EntityTypeRegistry::registerEntityType(const std::string& name, uint32_t entityType)
{
    if (m_entityTypesByNames.contains(name)) [[unlikely]]
    {
        spdlog::error("Entity name {} already registered with type {}", name,
                      m_entityTypesByNames.at(name));
    }
    else
    {
        spdlog::debug("Registering entity. name '{}', type {}", name, entityType);

        m_entityTypesByNames[name] = entityType;
        m_entityTypes.insert(entityType);
    }
}

GraphicsID EntityTypeRegistry::getHUDIcon(uint32_t entityType) const
{
    return m_icons.at(entityType);
}

std::string EntityTypeRegistry::getHUDDisplayName(uint32_t entityType) const
{
    return m_displayName.at(entityType);
}

void EntityTypeRegistry::registerHUDDisplayName(uint32_t entityType, const std::string& displayName)
{
    m_displayName[entityType] = displayName;
}

void EntityTypeRegistry::registerHUDIcon(uint32_t entityType, const GraphicsID& icon)
{
    m_icons[entityType] = icon;
}
