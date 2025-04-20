#ifndef COMPONENT_H
#define COMPONENT_H

namespace aion
{
class ComponentBase
{
  protected:
    inline static int counter_ = 0;
};

template <typename Derived> class Component : public ComponentBase
{
  public:
    static int type()
    {
        static int type = counter_++;
        return type;
    }
};
} // namespace aion

#endif