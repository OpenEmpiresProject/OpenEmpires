#ifndef SUBSYSTEM_H
#define SUBSYSTEM_H

namespace aion
{
    class SubSystem
    {
    public:
        virtual ~SubSystem() = default;
        virtual void init() = 0;
        virtual void shutdown() = 0;
    };
}

#endif