#ifndef INJECTED_REF_H
#define INJECTED_REF_H

#include "ServiceRegistry.h"
#include "Types.h"

namespace core
{
/**
 * @file LazyServiceRef.h
 * @brief Lightweight lazy-resolving service reference wrapper.
 *
 * `LazyServiceRef<T>` provides a dependency-injection type small convenience
 * wrapper that lazily queries the `ServiceRegistry` for a service of type `T`
 * the first time the wrapper is used. This allows users not to worry about
 * service availability and prevent maintaining a dependency tree.
 *
 * Important notes:
 * - Resolution happens on first access
 * - If the registry does not contain a service of type `T`, `getRef()` will
 *   return an empty `Ref<T>` and `operator->()` will return `nullptr`.
 * - Not thread-safety by default.
 *
 * @tparam T Service type to fetch from the ServiceRegistry.
 */
template <typename T> class LazyServiceRef
{
  public:
    /**
     * @brief Resolve the service (if needed) and return a raw pointer.
     *
     * This operator ensures the underlying `Ref<T>` is resolved via
     * `ServiceRegistry::getInstance().getService<T()` on first use and then
     * returns the cached raw pointer `m_rawPtr`.
     *
     * @return T* Pointer to the resolved service instance, or `nullptr` if the
     *             service was not found in the registry.
     */
    T* operator->() const
    {
        resolve();
        return m_rawPtr;
    }

    /**
     * @brief Resolve the service (if needed) and return the owning `Ref<T>`.
     *
     * Use this method when you need ownership semantics or want to validate
     * the presence of the service before accessing it.
     *
     * @return Ref<T> Owning reference to the service; may be empty if the
     *                service is not registered.
     */
    const Ref<T>& getRef() const
    {
        resolve();
        return m_ref;
    }

  private:
    void resolve() const
    {
        if (!m_ref)
        {
            m_ref = ServiceRegistry::getInstance().getService<T>();
            m_rawPtr = m_ref.get();
        }
    }

    mutable Ref<T> m_ref;
    mutable T* m_rawPtr = nullptr;
};

} // namespace core

#endif