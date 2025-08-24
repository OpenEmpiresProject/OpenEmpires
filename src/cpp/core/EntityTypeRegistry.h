#ifndef ENTITYTYPEREGISTRY_H
#define ENTITYTYPEREGISTRY_H

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

  private:
    std::unordered_map<std::string, uint32_t> m_entityTypesByNames;
    std::unordered_set<uint32_t> m_entityTypes;
};

} // namespace core

#endif
