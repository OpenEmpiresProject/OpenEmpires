#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include <cstddef>
#include <memory>
#include <stack>

namespace utils
{
template <typename T> class ObjectPool
{
  public:
    using Ptr = T*;

    ObjectPool() = delete;

    // Preallocate a fixed number of objects
    static void reserve(size_t count)
    {
        auto& storage = getThreadLocalStorage();
        for (size_t i = 0; i < count; ++i)
        {
            storage.pool.push(static_cast<Ptr>(::operator new(sizeof(T))));
        }
    }

    // Create or reuse an object with constructor arguments
    template <typename... Args> static Ptr acquire(Args&&... args)
    {
        auto& storage = getThreadLocalStorage();

        if (!storage.pool.empty())
        {
            Ptr obj = storage.pool.top();
            storage.pool.pop();
            new (obj) T(std::forward<Args>(args)...); // Placement new with constructor arguments
            return obj;
        }
        return new T(std::forward<Args>(args)...); // Allocate and construct directly
    }

    // Return the object to the pool
    static void release(Ptr obj)
    {
        if (obj != nullptr)
        {
            auto& storage = getThreadLocalStorage();
            storage.pool.push(obj);
        }
    }

    static size_t getSize()
    {
        auto& storage = getThreadLocalStorage();
        return storage.pool.size();
    }

    // // Free all objects in the pool
    // ~ObjectPool()
    // {
    //     while (!pool.empty())
    //     {
    //         pool.pop();
    //     }
    // }

  private:
    struct WarmUpInitializer
    {
        WarmUpInitializer()
        {
            spdlog::info("Warming up ObjectPool for type: {}", typeid(T).name());
            auto& storage = getThreadLocalStorage();
            storage.pool.reserve(100);
        }
    };

    struct Storage
    {
        std::stack<Ptr> pool;
    };

    static Storage& getThreadLocalStorage()
    {
        static thread_local Storage storage;
        return storage;
    }

    static thread_local WarmUpInitializer warmUpInitializer; // Ensure the pool is warmed up
};

} // namespace utils

#endif