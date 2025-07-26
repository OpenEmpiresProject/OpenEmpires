#ifndef PROPERTY_H
#define PROPERTY_H

#include <stdexcept>

namespace ion
{
class PropertyInitializer;

template <typename T> class Property
{
  public:
    // Implicit cast operator to T
    operator T() const
    {
        return value_;
    }

    const T& value() const
    {
        return value_;
    }
    bool isSet() const
    {
        return is_set_;
    }

  private:
    friend class PropertyInitializer;
    T value_{};
    bool is_set_ = false;
};

class PropertyInitializer
{
  protected:
    template <typename T> static void set(Property<T>& ability, const T& t)
    {
        if (ability.is_set_)
        {
            throw std::logic_error("Property already set!");
        }
        ability.value_ = t;
        ability.is_set_ = true;
    }
};
} // namespace ion

#endif // !PROPERTY_H
