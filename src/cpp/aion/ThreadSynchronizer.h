#ifndef THREADSYNCHRONIZER_H
#define THREADSYNCHRONIZER_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

namespace aion
{
    struct FrameData
    {
        int frameNumber = 0;
    };

    class ThreadSynchronizer
    {
    public:
        FrameData& getSenderFrameData()
        {
            return m_buffers[m_senderIndex.load()];
        }

        void waitForReceiver()
        {
            {
                std::unique_lock lock(m_mtx);
                m_cv.wait(lock, [this] { return !m_frameReady; });

                int temp = m_senderIndex.load(std::memory_order_relaxed);
                m_senderIndex.store(m_receiverIndex.load(std::memory_order_relaxed),
                               std::memory_order_relaxed);
                m_receiverIndex.store(temp, std::memory_order_relaxed);
                m_frameReady = true;
            }
           m_cv.notify_one();
        }

        FrameData& getReceiverFrameData()
        {
            return m_buffers[m_receiverIndex.load()];
        }

        void waitForSender()
        {
            {
                std::unique_lock lock(m_mtx);
                m_frameReady = false;
            }
            m_cv.notify_one(); // Wake up Simulator

            {
                std::unique_lock lock(m_mtx);
                m_cv.wait(lock, [this] { return m_frameReady; });
            }
        }

    private:
        FrameData m_buffers[2];
        std::atomic<int> m_senderIndex{0};
        std::atomic<int> m_receiverIndex{1};

        std::mutex m_mtx;
        std::condition_variable m_cv;
        bool m_frameReady = false;
    };
} // namespace aion


#endif