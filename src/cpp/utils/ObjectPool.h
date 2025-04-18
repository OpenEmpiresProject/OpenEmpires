#ifndef OBJECTPOOL_H
#define OBJECTPOOL_H

#include <stack>
#include <memory>
#include <cstddef>

namespace utils
{
    template<typename T>
    class ObjectPool {
    public:
        using Ptr = T*;

        ObjectPool() = default;

        // Preallocate a fixed number of objects
        void reserve(size_t count) {
            for (size_t i = 0; i < count; ++i) 
            {
                pool.push(static_cast<Ptr>(::operator new(sizeof(T))));
            }
            poolSize += count;
        }

        // Create or reuse an object
        Ptr acquire() {
            if (!pool.empty()) {
            Ptr obj = pool.top();
            pool.pop();
            return obj;
            }

            return static_cast<Ptr>(::operator new(sizeof(T)));
        }

        // Return the object to the pool
        void release(Ptr obj) {
            pool.push(obj);
        }

        // Total number of objects ever created
        size_t getPoolSize() const {
            return poolSize;
        }

        // Number of currently available objects
        size_t getFreeSize() const {
            return pool.size();
        }

        // Free all objects in the pool
        ~ObjectPool() {
            while (!pool.empty()) {
                pool.pop();
            }
        }

    private:
        std::stack<Ptr> pool;
        size_t poolSize = 0;
    };

    
    
}

#endif