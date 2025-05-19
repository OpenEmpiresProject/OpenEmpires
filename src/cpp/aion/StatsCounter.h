#ifndef STATSCOUNTER_H
#define STATSCOUNTER_H

#include <limits>
#include <optional>

namespace aion
{
template <typename T> class StatsCounter
{
  public:
    void addSample(const T& value)
    {
        m_count++;
        m_sum += value;
        if (value < m_min)
            m_min = value;
        if (value > m_max)
            m_max = value;
    }

    T average() const
    {
        return m_sum / static_cast<T>(m_count == 0 ? 1 : m_count);
    }

    T min() const
    {
        return m_min;
    }

    T max() const
    {
        return m_max;
    }

    size_t count() const
    {
        return m_count;
    }

    T sum() const
    {
        return m_sum;
    }

    void reset()
    {
        m_count = 0;
        m_sum = 0;
        m_min = std::numeric_limits<T>::max();
        m_max = std::numeric_limits<T>::lowest();
    }

    void resetIfCountIs(size_t count)
    {
        if (m_count == count)
        {
            reset();
        }
    }

  private:
    size_t m_count = 0;
    T m_sum = 0;
    T m_min = std::numeric_limits<T>::max();
    T m_max = std::numeric_limits<T>::lowest();
};
} // namespace aion

#endif