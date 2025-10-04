#include <gtest/gtest.h>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

#include "utils/ObjectPool.h"

namespace core 
{

template <int ID>
struct UniqueType 
{
    int value;
    UniqueType(int v = 0) : value(v) {}
    ~UniqueType() {}
};

template <int ID>
using GetUniqueType = UniqueType<ID>;

TEST(ObjectPoolTest, SingleThreadedAcquireRelease) 
{
    using MyType = GetUniqueType<1>;
    ObjectPool<MyType>::reserve(5);
    ASSERT_EQ(ObjectPool<MyType>::getSize(), 5);

    MyType* obj1 = ObjectPool<MyType>::acquire(10);
    ASSERT_NE(obj1, nullptr);
    ASSERT_EQ(obj1->value, 10);
    ASSERT_EQ(ObjectPool<MyType>::getSize(), 4);

    MyType* obj2 = ObjectPool<MyType>::acquire(20);
    ASSERT_NE(obj2, nullptr);
    ASSERT_EQ(obj2->value, 20);
    ASSERT_EQ(ObjectPool<MyType>::getSize(), 3);

    ObjectPool<MyType>::release(obj1);
    ASSERT_EQ(ObjectPool<MyType>::getSize(), 4);

    ObjectPool<MyType>::release(obj2);
    ASSERT_EQ(ObjectPool<MyType>::getSize(), 5);
}

TEST(ObjectPoolTest, SingleThreadedAcquireWithoutReserve) 
{
    using MyType = GetUniqueType<2>;

    ASSERT_EQ(ObjectPool<MyType>::getSize(), 0);
    MyType* obj1 = ObjectPool<MyType>::acquire(30);
    ASSERT_NE(obj1, nullptr);
    ASSERT_EQ(obj1->value, 30);
    ASSERT_EQ(ObjectPool<MyType>::getSize(), 0); // Should allocate directly

    ObjectPool<MyType>::release(obj1);
    ASSERT_EQ(ObjectPool<MyType>::getSize(), 1); // Released to local pool
}

TEST(ObjectPoolTest, SingleThreadedReleaseBeyondLocalCapacity) 
{
    using MyType = GetUniqueType<3>;

    for (size_t i = 0; i < ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE + 1; ++i) {
        ObjectPool<MyType>::release(new MyType(static_cast<int>(i)));
    }
    auto& globalStorage = ObjectPool<MyType>::getGlobalStorage();

    // Half of the local pool is returned to the global pool upon reaching the limit
    ASSERT_EQ(ObjectPool<MyType>::getSize(), ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE / 2 + 1);
    ASSERT_EQ(globalStorage.pool.size(), 5000);
}

TEST(ObjectPoolTest, MultiThreadedAcquireRelease) 
{
    using MyType = GetUniqueType<4>;

    const int numThreads = 1; // Hard to get a deterministic output with more than 1 thread 
    const int iterations = ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE + 1;
    std::vector<std::thread> threads;
    std::vector<MyType*> acquiredObjects[numThreads];

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, threadIndex = i]() {
            for (int j = 0; j < iterations; ++j) {
                MyType* obj = ObjectPool<MyType>::acquire(threadIndex * iterations + j);
                ASSERT_NE(obj, nullptr);
                ASSERT_EQ(obj->value, threadIndex * iterations + j);
                acquiredObjects[threadIndex].push_back(obj);
                std::this_thread::yield(); // Encourage context switching
            }
            for (MyType* obj : acquiredObjects[threadIndex]) {
                ObjectPool<MyType>::release(obj);
            }
            ASSERT_EQ(ObjectPool<MyType>::getSize(), 5001);

        });
    }
    for (auto& thread : threads) {
        thread.join();
    }

    ASSERT_EQ(ObjectPool<MyType>::getGlobalStorage().pool.size(), 5000);
    ASSERT_EQ(ObjectPool<MyType>::getThreadLocalStorage().pool.size(), 0);

    for (int j = 0; j < (ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE * 0.25); ++j) {
        MyType* obj = ObjectPool<MyType>::acquire(j); // Local pool is empty, should pull from global
        ASSERT_NE(obj, nullptr);
        ASSERT_EQ(obj->value, j);
        ObjectPool<MyType>::release(obj);
    }

    ASSERT_EQ(ObjectPool<MyType>::getGlobalStorage().pool.size(), 2500);
    ASSERT_EQ(ObjectPool<MyType>::getThreadLocalStorage().pool.size(), 2500);
}

TEST(ObjectPoolTest, GlobalPoolRefill) 
{
    using MyType = GetUniqueType<6>;

    const int numThreads = 2;
    const int iterations = ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE * 2;

    // Fill the global pool
    for (int i = 0; i < iterations; ++i) {
        auto& globalStorage = ObjectPool<MyType>::getGlobalStorage();
        std::lock_guard<std::mutex> lock(globalStorage.mutex);
        globalStorage.pool.push(new MyType(i));
    }

    std::vector<std::thread> threads;
    std::vector<MyType*> acquiredObjects[numThreads];

    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, threadIndex = i]() {
            for (int j = 0; j < iterations / numThreads; ++j) {
                MyType* obj = ObjectPool<MyType>::acquire(threadIndex * (iterations / numThreads) + j);
                ASSERT_NE(obj, nullptr);
                acquiredObjects[threadIndex].push_back(obj);
            }
            for (MyType* obj : acquiredObjects[threadIndex]) {
                ObjectPool<MyType>::release(obj);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    size_t totalLocalSize = 0;
    for (int i = 0; i < numThreads; ++i) {
        totalLocalSize += ObjectPool<MyType>::getSize();
    }

    auto& globalStorage = ObjectPool<MyType>::getGlobalStorage();
    std::lock_guard<std::mutex> lock(globalStorage.mutex);
    ASSERT_LE(totalLocalSize + globalStorage.pool.size(), static_cast<size_t>(iterations));
}

TEST(ObjectPoolTest, GlobalPoolOffload) 
{
    using MyType = GetUniqueType<7>;

    const int numThreads = 1;
    const int iterations = ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE * 2;

    // Acquire more than the local capacity in a single thread
    std::vector<MyType*> acquiredObjects;
    for (int i = 0; i < iterations; ++i) {
        acquiredObjects.push_back(ObjectPool<MyType>::acquire(i));
    }

    // Release all acquired objects in the same thread
    for (MyType* obj : acquiredObjects) {
        ObjectPool<MyType>::release(obj);
    }

    ASSERT_EQ(ObjectPool<MyType>::getSize(), ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE);

    auto& globalStorage = ObjectPool<MyType>::getGlobalStorage();
    ASSERT_EQ(globalStorage.pool.size(), iterations - ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE);
}

TEST(ObjectPoolTest, AcquireAfterGlobalOffload) 
{
    using MyType = GetUniqueType<8>;

    const int iterations = ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE * 2;
    std::vector<MyType*> acquiredObjects;
    for (int i = 0; i < iterations; ++i) {
        acquiredObjects.push_back(ObjectPool<MyType>::acquire(i)); // all objects are new
    }
    for (MyType* obj : acquiredObjects) {
        ObjectPool<MyType>::release(obj);
    }
    ASSERT_EQ(ObjectPool<MyType>::getSize(), ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE);
    ASSERT_EQ(ObjectPool<MyType>::getGlobalStorage().pool.size(), ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE);

    // Clear local pool to force refill from global
    auto& localStorage = ObjectPool<MyType>::getThreadLocalStorage();
    while (!localStorage.pool.empty()) {
        localStorage.pool.pop();
    }
    ASSERT_EQ(ObjectPool<MyType>::getSize(), 0);

    MyType* obj = ObjectPool<MyType>::acquire(100);
    ASSERT_NE(obj, nullptr);
    ASSERT_EQ(obj->value, 100);
    ASSERT_EQ(ObjectPool<MyType>::getSize(), ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE/2 - 1); // Object taken from refilled local pool
    ASSERT_EQ(ObjectPool<MyType>::getGlobalStorage().pool.size(), ObjectPool<MyType>::MAX_LOCAL_POOL_SIZE/2);

    ObjectPool<MyType>::release(obj);
}

TEST(ObjectPoolTest, ZeroReserveAcquireRelease) 
{
    using MyType = GetUniqueType<9>;

    ASSERT_EQ(ObjectPool<MyType>::getSize(), 0);
    MyType* obj = ObjectPool<MyType>::acquire(42);
    ASSERT_NE(obj, nullptr);
    ASSERT_EQ(obj->value, 42);
    ObjectPool<MyType>::release(obj);
    ASSERT_EQ(ObjectPool<MyType>::getSize(), 1);
}

} // namespace core
