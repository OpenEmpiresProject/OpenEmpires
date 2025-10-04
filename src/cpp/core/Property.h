#ifndef PROPERTY_H
#define PROPERTY_H

#include <stdexcept>

namespace core
{
class PropertyInitializer;

template <typename T> class Property
{
  public:
    Property()
    {
    }
    // Default value is not considered as a set value, hence m_isSet is false
    Property(const T& defaultValue) : m_value(defaultValue)
    {
    }

    // Implicit cast operator to T
    operator T() const
    {
        return m_value;
    }

    const T& value() const
    {
        return m_value;
    }
    bool isSet() const
    {
        return m_isSet;
    }

  private:
    friend class PropertyInitializer;
    T m_value{};
    bool m_isSet = false;
};

class PropertyInitializer
{
  protected:
    template <typename T> static void set(Property<T>& ability, const T& t)
    {
        if (ability.m_isSet)
        {
            throw std::logic_error("Property already set!");
        }
        ability.m_value = t;
        ability.m_isSet = true;
    }
};
} // namespace core

#endif // !PROPERTY_H
