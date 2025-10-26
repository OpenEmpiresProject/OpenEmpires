#ifndef ENTITYTYPEREGISTRY_H
#define ENTITYTYPEREGISTRY_H

#include "GraphicsRegistry.h"

#include <string>
#include <unordered_map>
#include <unordered_set>

namespace core
{
class EntityTypeRegistry
{
  public:
    bool isValid(uint32_t entityTye) const;
    bool isValid(const std::string& name) const;
    uint32_t getEntityType(const std::string& name) const;

    void registerEntityType(const std::string& name, uint32_t entityType);

    GraphicsID getHUDIcon(uint32_t entityType) const;
    std::string getHUDDisplayName(uint32_t entityType) const;
    uint32_t getUnitTypeHousingNeed(uint32_t entityType) const;
    uint32_t getNextAvailableEntityType() const;
    GraphicsID getCursorGraphic(const CursorType& type);
    bool isAUnit(uint32_t entityType) const;

    void registerHUDIcon(uint32_t entityType, const GraphicsID& icon);
    void registerHUDDisplayName(uint32_t entityType, const std::string& displayName);
    void registerUnitTypeHousingNeed(uint32_t entityType, uint32_t housingNeed);
    void registerUnitType(uint32_t entityType);
    void registerCursorGraphic(const CursorType& type, const GraphicsID& id);

  private:
    std::unordered_map<std::string, uint32_t> m_entityTypesByNames;
    std::unordered_set<uint32_t> m_entityTypes;
    std::unordered_map<uint32_t, GraphicsID> m_icons;
    std::unordered_map<uint32_t, std::string> m_displayName;
    std::unordered_map<uint32_t, uint32_t> m_unitTypeHousingNeeds;
    std::unordered_set<uint32_t> m_unitEntityTypes;
    std::unordered_map<CursorType, GraphicsID> m_cursorGraphics;
};

} // namespace core

#endif
