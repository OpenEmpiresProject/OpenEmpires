#ifndef SERVICEREGISTRY_H
#define SERVICEREGISTRY_H

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace aion
{
class ServiceRegistry
{
  public:
    static ServiceRegistry& getInstance()
    {
        static ServiceRegistry instance;
        return instance;
    }

    // Delete copy constructor and assignment operator to prevent copying
    ServiceRegistry(const ServiceRegistry&) = delete;
    ServiceRegistry& operator=(const ServiceRegistry&) = delete;

    template <typename T> void registerService(std::shared_ptr<T> service)
    {
        m_services[std::type_index(typeid(T))] = std::move(service);
    }

    template <typename T> std::shared_ptr<T> getService() const
    {
        auto it = m_services.find(std::type_index(typeid(T)));
        if (it == m_services.end())
        {
            throw std::runtime_error("Service not found");
        }
        return std::static_pointer_cast<T>(it->second);
    }

    template <typename T> bool hasService() const
    {
        return m_services.find(std::type_index(typeid(T))) != m_services.end();
    }

  private:
    ServiceRegistry()
    {
    }
    ~ServiceRegistry()
    {
    }

    std::unordered_map<std::type_index, std::shared_ptr<void>> m_services;
};

} // namespace aion

#endif