#ifndef CORE_VERSIONEDBUCKETSORTER_H
#define CORE_VERSIONEDBUCKETSORTER_H

#include "debug.h"

#include <vector>

namespace core
{
template<typename T>
class VersionedBucketSorter
{
public:
    VersionedBucketSorter() = default;
    VersionedBucketSorter(const size_t bucketsCount)
    {
        m_buckets.resize(bucketsCount);
    }

    void resize(const size_t bucketsCount)
    {
        m_buckets.resize(bucketsCount);
    }

    size_t getSize() const
    {
        return m_buckets.size();
    }
    
	uint64_t getVersion() const
    {
        return m_version;
    }

    void incrementVersion()
    {
        m_version++;
    }

    bool hasItems(size_t index) const
    {
        debug_assert(index < m_buckets.size(),
                     "Invalid index. Index [{}] should be less than the bucket size [{}]", index,
                     m_buckets.size());
        return m_buckets[index].version == m_version and m_buckets[index].items.empty() == false;
    }

    void emplaceItem(size_t index, const T& value)
    {
        ensureBucket(index).items.emplace_back(value); // copy
    }

    void emplaceItem(size_t index, T&& value)
    {
        ensureBucket(index).items.emplace_back(std::move(value)); // move
    }

    template <typename... Args> void emplaceItem(size_t index, Args&&... args)
    {
        ensureBucket(index).items.emplace_back(std::forward<Args>(args)...); // in-place construct
    }

    const std::vector<T>& getItems(size_t index) const
    {
        debug_assert(index < m_buckets.size(),
                     "Invalid index. Index [{}] should be less than the bucket size [{}]", index,
                     m_buckets.size());
        static std::vector<T> empty;
        return m_buckets[index].version == m_version ? m_buckets[index].items : empty;
    }

  private:
    struct Bucket
    {
        uint64_t version = 0;
        std::vector<T> items;
    };

    Bucket& ensureBucket(size_t index)
    {
        debug_assert(index < m_buckets.size(), "Invalid index [{}] < bucket size [{}]", index,
                     m_buckets.size());
        auto& bucket = m_buckets[index];
        if (bucket.version != m_version)
        {
            bucket.version = m_version;
            bucket.items.clear();
        }
        return bucket;
    }

  private:
    std::vector<Bucket> m_buckets;
    uint64_t m_version = 1;
};
} // namespace core

#endif // CORE_VERSIONEDBUCKETSORTER_H
