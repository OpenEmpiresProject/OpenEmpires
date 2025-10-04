#ifndef FLAT2DARRAY_H
#define FLAT2DARRAY_H

#include <cassert>
#include <vector>

namespace core
{
// A cache friendly 2D array implemented as a 1D array
template <typename T> class Flat2DArray
{
  public:
    Flat2DArray() : m_width(0), m_height(0)
    {
    }
    Flat2DArray(size_t width, size_t height, const T& defaultValue = T())
        : m_width(width), m_height(height), m_data(width * height, defaultValue)
    {
    }

    // Resize array with optional default fill value
    void resize(size_t width, size_t height, const T& value = T())
    {
        m_width = width;
        m_height = height;
        m_data.resize(width * height, value);
    }

    void clear()
    {
        m_width = m_height = 0;
        m_data.clear();
    }

    // Access with bounds checking in debug mode
    T& at(size_t x, size_t y)
    {
        assert(x < m_width && y < m_height);
        return m_data[y * m_width + x];
    }

    const T& at(size_t x, size_t y) const
    {
        assert(x < m_width && y < m_height);
        return m_data[y * m_width + x];
    }

    // Unchecked access (fast)
    T& operator()(size_t x, size_t y)
    {
        return m_data[y * m_width + x];
    }

    const T& operator()(size_t x, size_t y) const
    {
        return m_data[y * m_width + x];
    }

    void fill(const T& value)
    {
        std::fill(m_data.begin(), m_data.end(), value);
    }

    size_t width() const
    {
        return m_width;
    }
    size_t height() const
    {
        return m_height;
    }
    size_t size() const
    {
        return m_data.size();
    }

    T* data()
    {
        return m_data.data();
    }
    const T* data() const
    {
        return m_data.data();
    }

  private:
    size_t m_width = 0;
    size_t m_height = 0;
    std::vector<T> m_data;
};
} // namespace core

#endif