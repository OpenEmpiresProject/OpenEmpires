#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <chrono>
#include <numeric>
#include <vector>

namespace core
{

class FPSCounter
{
  public:
    FPSCounter(size_t averagingWindowFrames = 100)
        : m_averagingWindowFrames(averagingWindowFrames), m_frameTimeAccumulator(0.0f),
          m_frameSleepAccumulator(0.0f), m_frameCount(0), m_averageFPS(0.0f), m_averageSleep(0.0f),
          m_lastTime(std::chrono::steady_clock::now()),
          m_lastTotalSleepReturnedTime(std::chrono::steady_clock::now()), m_stableFPS(0.0f),
          m_lastStableUpdate(m_lastTime)
    {
    }

    void frame()
    {
        auto now = std::chrono::steady_clock::now();
        float delta = std::chrono::duration<float>(now - m_lastTime).count();
        m_lastTime = now;

        if (delta > 0.0f)
        {
            m_frameTimeAccumulator += delta;
            m_frameCount++;

            if (m_frameCount >= m_averagingWindowFrames)
            {
                m_averageFPS = static_cast<float>(m_averagingWindowFrames) / m_frameTimeAccumulator;
                m_averageSleep =
                    m_frameSleepAccumulator / static_cast<float>(m_averagingWindowFrames);
                m_frameTimeAccumulator = 0.0f;
                m_frameSleepAccumulator = 0.0f;
                m_frameCount = 0;
            }
        }
    }

    float getAverageFPS() const
    {
        return m_averageFPS;
    }

    float getFPS() const
    {
        if (m_frameCount > 0 && m_frameTimeAccumulator > 0.0f)
        {
            return static_cast<float>(m_frameCount) / m_frameTimeAccumulator;
        }
        return m_averageFPS; // Return the rolling average if not enough frames yet
    }

    bool isStable()
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<float>(now - m_lastStableUpdate).count() >= 1.0f)
        {
            m_stableFPS = getAverageFPS();
            m_lastStableUpdate = now;
            return true;
        }
        return false;
    }

    void sleptFor(int ms)
    {
        if (ms > 0.0f)
        {
            m_frameSleepAccumulator += ms;
        }
    }

    int getTotalSleep()
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<float>(now - m_lastTotalSleepReturnedTime).count() >= 1.0f)
        {
            m_lastReturnedTotalSleep = m_frameSleepAccumulator;
            m_lastTotalSleepReturnedTime = now;
            return true;
        }

        return m_lastReturnedTotalSleep;
    }

    float getAverageSleepMs() const
    {
        return m_averageSleep;
    }

  private:
    size_t m_averagingWindowFrames;
    float m_frameTimeAccumulator;
    float m_frameSleepAccumulator;
    float m_lastReturnedTotalSleep = 0.0f;
    std::chrono::steady_clock::time_point m_lastTotalSleepReturnedTime;

    size_t m_frameCount;
    float m_averageFPS;
    float m_averageSleep;
    std::chrono::steady_clock::time_point m_lastTime;
    float m_stableFPS;
    std::chrono::steady_clock::time_point m_lastStableUpdate;
};

} // namespace core

#endif // FPS_COUNTER_H