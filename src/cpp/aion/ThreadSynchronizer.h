#ifndef THREADSYNCHRONIZER_H
#define THREADSYNCHRONIZER_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

namespace aion
{
template <typename T> class ThreadSynchronizer
{
  public:
    T& getSenderFrameData()
    {
        return m_buffers[m_senderIndex.load()];
    }

    T& getReceiverFrameData()
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
            m_cv.wait(lock, [this] { return m_frameReady || m_shutdown; });
        }
    }

    void waitForReceiver(std::function<void()> synchronizedBlock = []() {})
    {
        {
            std::unique_lock lock(m_mtx);
            m_cv.wait(lock, [this] { return !m_frameReady || m_shutdown; });

            if (m_shutdown)
                return;

            synchronizedBlock();

            int temp = m_senderIndex.load(std::memory_order_relaxed);
            m_senderIndex.store(m_receiverIndex.load(std::memory_order_relaxed),
                                std::memory_order_relaxed);
            m_receiverIndex.store(temp, std::memory_order_relaxed);
            m_frameReady = true;
        }
        m_cv.notify_one();
    }

    void shutdown()
    {
        {
            std::unique_lock lock(m_mtx);
            m_shutdown = true;
        }
        m_cv.notify_all(); // Wake up all waiting threads
    }

  private:
    T m_buffers[2];
    std::atomic<int> m_senderIndex{0};
    std::atomic<int> m_receiverIndex{1};
    std::atomic<bool> m_shutdown{false};
    std::mutex m_mtx;
    std::condition_variable m_cv;
    bool m_frameReady = false;
};
} // namespace aion

#endif