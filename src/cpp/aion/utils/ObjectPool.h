#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include "utils/Logger.h"

#include <cstddef>
#include <memory>
#include <mutex>
#include <sstream>
#include <stack>
#include <thread>

namespace aion
{
template <typename T> class ObjectPool
{
  public:
    constexpr static size_t MAX_LOCAL_POOL_SIZE = 10000;

    using Ptr = T*;

    struct GlobalStorage
    {
        std::stack<Ptr> pool;
        std::mutex mutex;
    };

    static GlobalStorage& getGlobalStorage()
    {
        static GlobalStorage storage;
        return storage;
    }

    static std::string threadIdStr()
    {
        std::ostringstream oss;
        oss << std::this_thread::get_id();
        return oss.str();
    }

    struct Storage
    {
        std::stack<Ptr> pool;
    };

    static Storage& getThreadLocalStorage()
    {
        static thread_local Storage storage;
        return storage;
    }

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

        if (storage.pool.empty())
        {
            auto& global = getGlobalStorage();
            std::lock_guard<std::mutex> lock(global.mutex);

            size_t refillCount = std::min(MAX_LOCAL_POOL_SIZE, global.pool.size() / 2);
            for (size_t i = 0; i < refillCount; ++i)
            {
                storage.pool.push(global.pool.top());
                global.pool.pop();
            }

            if (refillCount > 0)
            {
                spdlog::debug("Thread [{}] refilled {} objects from global pool.", threadIdStr(),
                              refillCount);
            }
        }

        if (!storage.pool.empty())
        {
            Ptr obj = storage.pool.top();
            storage.pool.pop();
            new (obj) T(std::forward<Args>(args)...);
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
            if (storage.pool.size() < MAX_LOCAL_POOL_SIZE)
            {
                storage.pool.push(obj);
            }
            else
            {
                constexpr size_t CHUNK_SIZE = MAX_LOCAL_POOL_SIZE / 2;

                auto& global = getGlobalStorage();
                std::lock_guard<std::mutex> lock(global.mutex);

                for (size_t i = 0; i < CHUNK_SIZE; ++i)
                {
                    global.pool.push(storage.pool.top());
                    storage.pool.pop();
                }

                spdlog::debug("Thread [{}] offloaded {} objects from local pool to global pool.",
                              threadIdStr(), CHUNK_SIZE);

                storage.pool.push(obj);
            }
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

    static thread_local WarmUpInitializer s_warmUpInitializer; // Ensure the pool is warmed up
};

} // namespace aion

#endif