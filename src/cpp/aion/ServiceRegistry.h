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
        services_[std::type_index(typeid(T))] = std::move(service);
    }

    template <typename T> std::shared_ptr<T> getService() const
    {
        auto it = services_.find(std::type_index(typeid(T)));
        if (it == services_.end())
        {
            throw std::runtime_error("Service not found");
        }
        return std::static_pointer_cast<T>(it->second);
    }

    template <typename T> bool hasService() const
    {
        return services_.find(std::type_index(typeid(T))) != services_.end();
    }

  private:
    ServiceRegistry()
    {
    }
    ~ServiceRegistry()
    {
    }

    std::unordered_map<std::type_index, std::shared_ptr<void>> services_;
};

} // namespace aion

#endif