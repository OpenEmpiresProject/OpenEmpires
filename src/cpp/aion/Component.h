#ifndef COMPONENT_H
#define COMPONENT_H

namespace aion
{
    class Component
    {
    public:
        virtual ~Component() = default;
        virtual void init() = 0;
        virtual void shutdown() = 0;
    };
}

#endif