#ifndef FPS_COUNTER_H
#define FPS_COUNTER_H

#include <chrono>
#include <numeric>
#include <vector>

namespace aion
{

class FPSCounter
{
  public:
    FPSCounter(size_t averagingWindowFrames = 100)
        : averagingWindowFrames_(averagingWindowFrames), frameTimeAccumulator_(0.0f),
          frameSleepAccumulator_(0.0f), frameCount_(0), averageFPS_(0.0f),
          lastTime_(std::chrono::steady_clock::now()),
          lastTotalSleepReturnedTime(std::chrono::steady_clock::now()), stableFPS_(0.0f),
          lastStableUpdate_(lastTime_)
    {
    }

    void frame()
    {
        auto now = std::chrono::steady_clock::now();
        float delta = std::chrono::duration<float>(now - lastTime_).count();
        lastTime_ = now;

        if (delta > 0.0f)
        {
            frameTimeAccumulator_ += delta;
            frameCount_++;

            if (frameCount_ >= averagingWindowFrames_)
            {
                averageFPS_ = static_cast<float>(averagingWindowFrames_) / frameTimeAccumulator_;
                averageSleep_ = frameSleepAccumulator_ / static_cast<float>(averagingWindowFrames_);
                frameTimeAccumulator_ = 0.0f;
                frameSleepAccumulator_ = 0.0f;
                frameCount_ = 0;
            }
        }
    }

    float getAverageFPS() const
    {
        return averageFPS_;
    }

    float getFPS() const
    {
        if (frameCount_ > 0 && frameTimeAccumulator_ > 0.0f)
        {
            return static_cast<float>(frameCount_) / frameTimeAccumulator_;
        }
        return averageFPS_; // Return the rolling average if not enough frames yet
    }

    bool isStable()
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<float>(now - lastStableUpdate_).count() >= 1.0f)
        {
            stableFPS_ = getAverageFPS();
            lastStableUpdate_ = now;
            return true;
        }
        return false;
    }

    void sleptFor(int ms)
    {
        if (ms > 0.0f)
        {
            frameSleepAccumulator_ += ms;
        }
    }

    int getTotalSleep()
    {
        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration<float>(now - lastTotalSleepReturnedTime).count() >= 1.0f)
        {
            lastReturnedTotalSleep_ = frameSleepAccumulator_;
            lastTotalSleepReturnedTime = now;
            return true;
        }

        return lastReturnedTotalSleep_;
    }

    float getAverageSleepMs() const
    {
        return averageSleep_;
    }

  private:
    size_t averagingWindowFrames_;
    float frameTimeAccumulator_;
    float frameSleepAccumulator_;
    float lastReturnedTotalSleep_ = 0.0f;
    std::chrono::steady_clock::time_point lastTotalSleepReturnedTime;

    size_t frameCount_;
    float averageFPS_;
    float averageSleep_;
    std::chrono::steady_clock::time_point lastTime_;
    float stableFPS_;
    std::chrono::steady_clock::time_point lastStableUpdate_;
};

} // namespace aion

#endif // FPS_COUNTER_H