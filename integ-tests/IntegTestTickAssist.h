#ifndef INTEGTESTTICKASSIST_H
#define INTEGTESTTICKASSIST_H

#include "EventHandler.h"

#include <mutex>

namespace game
{
    class IntegTestTickAssist : public core::EventHandler
    {
    public:
        IntegTestTickAssist(/* args */);
        ~IntegTestTickAssist();

        void aquireLock();
        void releaseLock();

    private:
        void onTick(const core::Event& e);
    
        std::mutex m_syncMutex;
        std::unique_lock<std::mutex> m_lock;
    };
} // namespace game


#endif