#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <memory>
#include <list>
#include "EventLoopListener.h"
#include "Event.h"
#include "SubSystem.h"
#include <thread>

namespace aion
{
    class EventLoop : public SubSystem {
    public:
        EventLoop();
        ~EventLoop();

        void registerListener(std::unique_ptr<EventLoopListener> listener);
        // TODO: Might need to refactor this to use a shared_ptr or weak_ptr
        void deregisterListener(EventLoopListener* listener);
        int getListenersCount() const { return listeners.size(); }
        

    private:
        // SubSystem methods
        void init() override;
        void run();
        void shutdown() override;
        
        std::list<std::unique_ptr<EventLoopListener>> listeners;
        std::thread eventLoopThread;
    };
     
}

#endif // EVENTLOOP_H